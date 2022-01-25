/**
 * Copyright (C) 2021 by The Toxfs Project Contributers
 *
 * This file is part of Toxfs.
 *
 * Toxfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toxfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Toxfs.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "toxfs/tox/tox.hh"
#include "toxfs/tox/tox_error.hh"
#include "toxfs/exception.hh"
#include "toxfs/logging.hh"
#include "toxfs/util/message_queue.hh"
#include "toxfs/util/compile_utils.hh"
#include "toxfs/util/string_helpers.hh"

#include "toxfs_priv/tox/tox_if_impl.hh"
#include "toxfs_priv/tox/tox_if_convert.hh"

#include <tox/tox.h>

#include <fmt/ranges.h>

#include <gsl/gsl_util>

#include <string_view>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <cassert>
#include <fstream>
#include <memory>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <queue>
#include <optional>

namespace toxfs::tox
{

static_assert(TOX_PUBLIC_KEY_SIZE == sizeof(public_key_t), "Public Key Size Mismatch");
static_assert(TOX_NOSPAM_SIZE == sizeof(nospam_t), "No Spam Size Mismatch");
static_assert(TOX_ADDRESS_SIZE == sizeof(address_t), "Address Size Mismatch");

namespace detail
{
#ifdef TOXFS_ALL_TOXCORE_LOGS
constexpr auto k_toxcore_min_level = TOX_LOG_LEVEL_TRACE;
#else
constexpr auto k_toxcore_min_level = TOX_LOG_LEVEL_INFO;
#endif

static void log_callback(Tox *, TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func,
        const char *message, void *) noexcept
{
    if (level < k_toxcore_min_level)
        return;

    const char *level_str = "UNKNOWN";
    switch (level)
    {
        case TOX_LOG_LEVEL_TRACE: level_str = "TRACE"; break;
        case TOX_LOG_LEVEL_DEBUG: level_str = "DEBUG"; break;
        case TOX_LOG_LEVEL_INFO: level_str = "INFO"; break;
        case TOX_LOG_LEVEL_WARNING: level_str = "WARNING"; break;
        case TOX_LOG_LEVEL_ERROR: level_str = "ERROR"; break;
    }

    TOXFS_LOG_DEBUG("[toxcore {} {}:{}] {}: {}", func, file, line, level_str, message);
}

template <typename T, typename ErrEnum>
void set_promise_from_tox(std::promise<T>& promise_ref, T value, ErrEnum err, const char* err_msg)
{
    if (!err)
    {
        promise_ref.set_value(value);
    }
    else
    {
        promise_ref.set_exception(std::make_exception_ptr(
                TOXFS_EXCEPTION(tox::tox_error, err_msg, err)));
    }
}

}  // namespace detail

struct tox_t::impl_t
{
    message_queue<int, 16> mq_{};
    Tox *tox_ = nullptr;

    tox_config_t config_;
    std::shared_ptr<tox_if_impl> if_impl_ptr_;
    send_queue_t& send_queue_ref_;
    recv_queue_t& recv_queue_ref_;

    struct chunk_requests_t
    {
        long num_in_flight{0};
        long max_in_flight{16};
        std::chrono::steady_clock::time_point last_update{std::chrono::steady_clock::now()};
        std::queue<file_chunk_request_t> requests;
    };
    std::unordered_map<unique_file_id_t, chunk_requests_t> chunk_requests_;

    std::thread loop_thread_;

    explicit impl_t(tox_config_t const& config);
    ~impl_t() noexcept;

    impl_t(impl_t const&) = delete;
    impl_t& operator=(impl_t const&) = delete;

    void setup_callbacks();

    void loop();

    /* Tox Callbacks */

    /* Friend */
    void on_friend_request(const uint8_t *public_key, const uint8_t *msg, size_t msg_len);

    void on_friend_msg(uint32_t fr_num, TOX_MESSAGE_TYPE type, const uint8_t *msg, size_t msg_len);

    void on_friend_name(uint32_t fr_num, const uint8_t *name, size_t name_len);

    void on_friend_status(uint32_t fr_num, const uint8_t *msg, size_t msg_len);

    void on_friend_conn_status(uint32_t fr_num, TOX_CONNECTION conn_status);

    /* Group */
    void on_group_invite(uint32_t fr_num, TOX_CONFERENCE_TYPE type, const uint8_t *cookie, size_t cookie_len);

    void on_group_title(uint32_t gr_num, uint32_t peer_num, const uint8_t *title, size_t title_len);

    void on_group_msg(uint32_t gr_num, uint32_t peer_num, TOX_MESSAGE_TYPE type, const uint8_t *msg, size_t msg_len);

    void on_group_peers_changed(uint32_t gr_num);

    void on_group_peer_name(uint32_t gr_num, uint32_t peer_num, const uint8_t *name, size_t name_len);

    /* File Transfer */

    void on_file_control(uint32_t fr_num, uint32_t file_num, TOX_FILE_CONTROL file_ctrl);

    void on_file_recv(uint32_t fr_num, uint32_t file_num, uint32_t kind, uint64_t file_size,
        const uint8_t *filename, size_t filename_len);

    void on_file_chunk_request(uint32_t fr_num, uint32_t file_num, uint64_t position, size_t length);

    void on_file_chunk(uint32_t fr_num, uint32_t file_num, uint64_t position, const uint8_t *data, size_t data_len);

    /* Send Handlers */

    void send_msg_(send_msg_get_conn_status_t&& msg);
    void send_msg_(send_msg_accept_fr_req_t&& msg);
    void send_msg_(send_msg_fr_message_t&& msg);
    void send_msg_(send_msg_file_send_t&& msg);
    void send_msg_(send_msg_file_control_t&& msg);
    void send_msg_(send_msg_file_chunk_t&& msg);
    void send_msg_(send_msg_savedata_t&& msg);

    /* Misc */

    void report_file_err_(unique_file_id_t id, tox_error error);

    void check_chunk_requests_(std::optional<unique_file_id_t> id);

    /**
     * Helper for binding a tox callback by passing this of impl_t as user_data
     * and then cast it back and call the corresponding member function after.
     */
    template <typename T> struct callback_t;
    template <typename ...Args>
    struct callback_t<void(tox_t::impl_t::*)(Args...)>
    {
        template<void(tox_t::impl_t::*mem_fn)(Args...)>
        static void callback(Tox* tox, Args... args, void *user_data) noexcept
        {
            impl_t *pThis = reinterpret_cast<tox_t::impl_t*>(user_data);
            assert(tox == pThis->tox_);
            if (tox == pThis->tox_)
            {
                try
                {
                    (pThis->*mem_fn)(args...);
                }
                catch (std::exception const& e)
                {
                    TOXFS_LOG_ERROR("Uncaught exception while handling tox callback: {}", e.what());
                }
            }
            else
            {
                TOXFS_LOG_ERROR("Unmatched callback pointer: {} != {}",
                    static_cast<void*>(tox), static_cast<void*>(pThis->tox_));
            }
        }
    };
};

using impl_t = tox_t::impl_t;

impl_t::impl_t(tox_config_t const& config)
    : config_(config)
    , if_impl_ptr_(std::make_shared<tox_if_impl>())
    , send_queue_ref_(if_impl_ptr_->get_send_queue())
    , recv_queue_ref_(if_impl_ptr_->get_recv_queue())
{
    TOX_ERR_OPTIONS_NEW options_err;
    Tox_Options *options = nullptr;
    options = tox_options_new(&options_err);
    if (!options)
        throw TOXFS_EXCEPTION(tox::tox_error, "tox_options_new failed", options_err);

    std::vector<char> savedata;
    if (!config_.save_file.empty())
    {
        if (std::filesystem::exists(config_.save_file))
        {
            std::ifstream s{config_.save_file, std::ios_base::in | std::ios_base::binary};
            s.seekg(0, std::ios_base::end);
            savedata.resize(static_cast<size_t>(s.tellg()));
            s.seekg(0, std::ios_base::beg);
            s.read(savedata.data(), static_cast<std::streamsize>(savedata.size()));
            tox_options_set_savedata_data(options, reinterpret_cast<uint8_t*>(savedata.data()), savedata.size());
            tox_options_set_savedata_type(options, TOX_SAVEDATA_TYPE_TOX_SAVE);
        }
        else
        {
            TOXFS_LOG_WARNING("tox savedata file does not exist ({}), save to create it", config_.save_file.native());
        }
    }

    auto free_options = gsl::finally([options]()
    {
        tox_options_free(options);
    });

    tox_options_set_log_callback(options, detail::log_callback);

    TOX_ERR_NEW new_err;
    tox_ = tox_new(options, &new_err);
    if (!tox_)
        throw TOXFS_EXCEPTION(tox::tox_error, "tox_new failed", new_err);
}

impl_t::~impl_t() noexcept
{
    if (tox_)
    {
        tox_kill(tox_);
        tox_ = nullptr;
    }
}

void impl_t::setup_callbacks()
{
    tox_callback_friend_request(tox_,
        callback_t<decltype(&impl_t::on_friend_request)>::callback<&impl_t::on_friend_request>);

    tox_callback_friend_message(tox_,
        callback_t<decltype(&impl_t::on_friend_msg)>::callback<&impl_t::on_friend_msg>);

    tox_callback_friend_name(tox_,
        callback_t<decltype(&impl_t::on_friend_name)>::callback<&impl_t::on_friend_name>);

    tox_callback_friend_status_message(tox_,
        callback_t<decltype(&impl_t::on_friend_status)>::callback<&impl_t::on_friend_status>);

    tox_callback_friend_connection_status(tox_,
        callback_t<decltype(&impl_t::on_friend_conn_status)>::callback<&impl_t::on_friend_conn_status>);

    tox_callback_conference_invite(tox_,
        callback_t<decltype(&impl_t::on_group_invite)>::callback<&impl_t::on_group_invite>);

    tox_callback_conference_title(tox_,
        callback_t<decltype(&impl_t::on_group_title)>::callback<&impl_t::on_group_title>);

    tox_callback_conference_message(tox_,
        callback_t<decltype(&impl_t::on_group_msg)>::callback<&impl_t::on_group_msg>);

    tox_callback_conference_peer_list_changed(tox_,
        callback_t<decltype(&impl_t::on_group_peers_changed)>::callback<&impl_t::on_group_peers_changed>);

    tox_callback_conference_peer_name(tox_,
        callback_t<decltype(&impl_t::on_group_peer_name)>::callback<&impl_t::on_group_peer_name>);

    tox_callback_file_recv_control(tox_,
        callback_t<decltype(&impl_t::on_file_control)>::callback<&impl_t::on_file_control>);
    tox_callback_file_recv(tox_,
        callback_t<decltype(&impl_t::on_file_recv)>::callback<&impl_t::on_file_recv>);

    tox_callback_file_chunk_request(tox_,
        callback_t<decltype(&impl_t::on_file_chunk_request)>::callback<&impl_t::on_file_chunk_request>);

    tox_callback_file_recv_chunk(tox_,
        callback_t<decltype(&impl_t::on_file_chunk)>::callback<&impl_t::on_file_chunk>);
}

void impl_t::loop()
{
    setup_callbacks();

    struct bootstrap_node
    {
        std::string url;
        uint16_t port;
        std::string key;
    };

    // TODO: put this elsewhere
    auto nodes =
    {
        bootstrap_node {
            "tox.abilinski.com", 33445,
            "10C00EB250C3233E343E2AEBA07115A5C28920E9C8D29492F6D00B29049EDC7E"
        },
        bootstrap_node {
            "tox.initramfs.io", 33445,
            "3F0A45A268367C1BEA652F258C85F4A66DA76BCAA667A49E770BCC4917AB6A25"
        },
    };

    bool ok = false;
    for (auto const& [url, port, key] : nodes)
    {
        TOXFS_LOG_INFO("Bootstrapping using: {}:{}", url, port);

        auto key_bin = hex_string_to_binary(key);
        TOX_ERR_BOOTSTRAP err;
        if (tox_bootstrap(tox_, url.c_str(), port, reinterpret_cast<uint8_t*>(key_bin.data()), &err))
        {
            ok = true;
        }
        else
        {
            TOXFS_LOG_WARNING("Bootstrapping with {}:{} failed: {}", url, port, err);
        }
    }

    if (!ok)
    {
        TOXFS_LOG_WARNING("Bootstrapping failed on all nodes! Only local will work.");
    }


    address_t addr;
    tox_self_get_address(tox_, reinterpret_cast<uint8_t*>(&addr));

    TOXFS_LOG_INFO("My tox address is: {:02x}", fmt::join(addr.bytes, ""));

    TOXFS_LOG_DEBUG("Tox address breakdown: public_key: {:02x} nospam: {:02x} checksum: {:02x}",
        fmt::join(addr.public_key(), ""),
        fmt::join(addr.nospam(), ""),
        fmt::join(addr.checksum(), ""));

    auto name = std::string_view{"toxfs daemon"};
    tox_self_set_name(tox_, reinterpret_cast<uint8_t const*>(name.data()), name.size(), nullptr);

    std::chrono::steady_clock::time_point start_time;
    while (true)
    {
        start_time = std::chrono::steady_clock::now();
        tox_iterate(tox_, this);
        auto end_time = start_time + std::chrono::milliseconds{tox_iteration_interval(tox_)};
        check_chunk_requests_(std::nullopt);
        auto opt_msg = send_queue_ref_.pop_until_time(end_time);
        if (opt_msg)
        {
            try
            {
                std::visit([this](auto&& msg) { send_msg_(std::move(msg)); }, *opt_msg);
            }
            catch (toxfs::exception const& e)
            {
                TOXFS_LOG_ERROR("Exception while sending message: {}", e.what());
            }
            catch (std::exception const& e)
            {
                TOXFS_LOG_ERROR("Exception while sending message: {}", e.what());
            }
        }
        std::this_thread::sleep_until(end_time);
    }
}

void impl_t::on_friend_request(const uint8_t *public_key, const uint8_t *msg, size_t msg_len)
{
    const public_key_t& public_key_arr = *reinterpret_cast<const public_key_t*>(public_key);
    std::string_view msg_str{reinterpret_cast<const char*>(msg), msg_len};
    TOXFS_LOG_DEBUG("on_friend_request from {:x}: {} (len = {})", fmt::join(public_key_arr, ""), msg_str, msg_len);

    recv_queue_ref_.push(recv_msg_fr_request_t { public_key_arr, std::string{msg_str} });
}

void impl_t::on_friend_msg(uint32_t fr_num, TOX_MESSAGE_TYPE type, const uint8_t *msg, size_t msg_len)
{
    std::string_view msg_str{reinterpret_cast<const char*>(msg), msg_len};
    TOXFS_LOG_DEBUG("on_friend_msg from #{}: (type {}) {} (len = {})", fr_num, int(type), msg_str, msg_len);

    recv_queue_ref_.push(recv_msg_fr_message_t { friend_id_t{fr_num}, std::string{msg_str} });
}

void impl_t::on_friend_name(uint32_t fr_num, const uint8_t *name, size_t name_len)
{
    std::string_view name_str{reinterpret_cast<const char*>(name), name_len};
    TOXFS_LOG_DEBUG("on_friend_name from #{}: {} (len = {})", fr_num, name_str, name_len);

    recv_queue_ref_.push(recv_msg_fr_name_t { friend_id_t{fr_num}, std::string{name_str} });
}

void impl_t::on_friend_status(uint32_t fr_num, const uint8_t *msg, size_t msg_len)
{
    std::string_view msg_str{reinterpret_cast<const char*>(msg), msg_len};
    TOXFS_LOG_DEBUG("on_friend_status from #{}: {} (len = {})", fr_num, msg_str, msg_len);

    recv_queue_ref_.push(recv_msg_fr_status_t { friend_id_t{fr_num}, std::string{msg_str} });
}

void impl_t::on_friend_conn_status(uint32_t fr_num, TOX_CONNECTION conn_status)
{
    TOXFS_LOG_DEBUG("on_friend_conn_status from #{}: {}", fr_num, int(conn_status));

    // TODO
}

void impl_t::on_group_invite(uint32_t fr_num, TOX_CONFERENCE_TYPE type, const uint8_t* cookie, size_t cookie_len)
{
    std::string_view cookie_str{reinterpret_cast<const char*>(cookie), cookie_len};
    TOXFS_LOG_DEBUG("on_group_invite from #{}: (type {}) {} (len = {}))", fr_num, int(type), cookie_str, cookie_len);
}

void impl_t::on_group_title(uint32_t gr_num, uint32_t peer_num, const uint8_t *title, size_t title_len)
{
    std::string_view title_str{reinterpret_cast<const char*>(title), title_len};
    TOXFS_LOG_DEBUG("on_group_title from #{} #{}: {} (len = {})", gr_num, peer_num, title_str, title_len);
}

void impl_t::on_group_msg(uint32_t gr_num, uint32_t peer_num, TOX_MESSAGE_TYPE type, const uint8_t *msg, size_t msg_len)
{
    std::string_view msg_str{reinterpret_cast<const char*>(msg), msg_len};
    TOXFS_LOG_DEBUG("on_group_msg from #{} #{}: (type {}) {} (len = {})", gr_num, peer_num, int(type), msg_str, msg_len);
}

void impl_t::on_group_peers_changed(uint32_t gr_num)
{
    TOXFS_LOG_DEBUG("on_group_changed from #{}", gr_num);
}

void impl_t::on_group_peer_name(uint32_t gr_num, uint32_t peer_num, const uint8_t *name, size_t name_len)
{
    std::string_view name_str{reinterpret_cast<const char*>(name), name_len};
    TOXFS_LOG_DEBUG("on_group_peer_name from #{} #{}: {} (len = {})", gr_num, peer_num, name_str, name_len);
}

void impl_t::on_file_control(uint32_t fr_num, uint32_t file_num, TOX_FILE_CONTROL file_ctrl)
{
    TOXFS_LOG_DEBUG("on_file_control from #{}: file #{} ctrl {}", fr_num, file_num, int(file_ctrl));

    recv_queue_ref_.push(recv_msg_file_control_t { unique_file_id_t{fr_num, file_num}, from_tox::convert(file_ctrl) });
}

void impl_t::on_file_recv(uint32_t fr_num, uint32_t file_num, uint32_t kind, uint64_t file_size,
    const uint8_t *filename, size_t filename_len)
{
    std::string_view filename_str{reinterpret_cast<const char*>(filename), filename_len};
    TOXFS_LOG_DEBUG("on_file_recv from #{}: file #{} {} size {} kind {}", fr_num, file_num,
        filename_str, file_size, kind);

    if (kind == TOX_FILE_KIND_AVATAR)
    {
        TOXFS_LOG_DEBUG("on_file_recv ignoring avatar file");
        return;
    }

    recv_queue_ref_.push(recv_msg_file_receive_t { unique_file_id_t{fr_num, file_num}, {std::string{filename_str}, file_size} });
}

void impl_t::on_file_chunk_request(uint32_t fr_num, uint32_t file_num, uint64_t position, size_t length)
{
    // TOXFS_LOG_DEBUG("on_file_chunk_request from #{}: file #{} position {} len {}", fr_num, file_num, position, length);

    unique_file_id_t file_id{fr_num, file_num};

    auto& req = chunk_requests_[file_id];

    req.requests.push(file_chunk_request_t{position, length});
    req.last_update = std::chrono::steady_clock::now();

    check_chunk_requests_(file_id);
}

void impl_t::on_file_chunk(uint32_t fr_num, uint32_t file_num, uint64_t position, const uint8_t *data, size_t data_len)
{
    // TOXFS_LOG_DEBUG("on_file_chunk from #{}: file #{} position {} len {}", fr_num, file_num, position, data_len);

    buffer_t buf{data_len};
    std::memcpy(buf.data(), data, data_len);
    buf.set_size(data_len);
    recv_queue_ref_.push(recv_msg_file_chunk_t { unique_file_id_t{fr_num, file_num}, {position, std::move(buf)} });
}

void impl_t::send_msg_(send_msg_get_conn_status_t&& /*msg*/)
{
    // TODO
}

void impl_t::send_msg_(send_msg_accept_fr_req_t&& msg)
{
    TOX_ERR_FRIEND_ADD err;
    uint32_t fr_id = tox_friend_add_norequest(tox_, reinterpret_cast<uint8_t const*>(msg.public_key.data()), &err);

    detail::set_promise_from_tox(msg.promise, friend_id_t{fr_id}, err, "tox_friend_add_norequest failed");
}

void impl_t::send_msg_(send_msg_fr_message_t&& msg)
{
    TOX_ERR_FRIEND_SEND_MESSAGE err = TOX_ERR_FRIEND_SEND_MESSAGE_OK;
    auto msg_id = tox_friend_send_message(tox_, msg.id.id, TOX_MESSAGE_TYPE_NORMAL,
            reinterpret_cast<uint8_t const*>(msg.message.data()), msg.message.size(), &err);

    detail::set_promise_from_tox(msg.promise, message_id_t{msg_id}, err, "tox_friend_send_message failed");
}

void impl_t::send_msg_(send_msg_file_send_t&& msg)
{
    TOX_ERR_FILE_SEND err = TOX_ERR_FILE_SEND_OK;
    auto file_id = tox_file_send(tox_, msg.id.id, TOX_FILE_KIND_DATA, msg.info.filesize, nullptr,
            reinterpret_cast<uint8_t const*>(msg.info.filename.data()), msg.info.filename.size(), &err);

    auto uniq_id = unique_file_id_t{msg.id, file_id_t{file_id}};
    detail::set_promise_from_tox(msg.promise, uniq_id, err, "tox_file_send failed");
}

void impl_t::send_msg_(send_msg_file_control_t&& msg)
{
    TOX_ERR_FILE_CONTROL err = TOX_ERR_FILE_CONTROL_OK;
    bool ok = tox_file_control(tox_, msg.id.friend_id.id, msg.id.file_id.id, to_tox::convert(msg.control), &err);

    if (!ok)
    {
        report_file_err_(msg.id, TOXFS_EXCEPTION(tox::tox_error, "tox_file_control failed", err));
    }
}

void impl_t::send_msg_(send_msg_file_chunk_t&& msg)
{
    TOX_ERR_FILE_SEND_CHUNK err = TOX_ERR_FILE_SEND_CHUNK_OK;
    bool ok = tox_file_send_chunk(tox_, msg.id.friend_id.id, msg.id.file_id.id, msg.chunk.position,
            reinterpret_cast<uint8_t const*>(msg.chunk.data.data()), msg.chunk.data.size(), &err);

    if (!ok)
    {
        report_file_err_(msg.id, TOXFS_EXCEPTION(tox::tox_error, "tox_file_send_chunk failed", err));
    }

    auto& req = chunk_requests_[msg.id];
    req.last_update = std::chrono::steady_clock::now();
    req.num_in_flight--;

    check_chunk_requests_(msg.id);
}

void impl_t::send_msg_(send_msg_savedata_t&&)
{
    std::vector<char> data;
    data.resize(tox_get_savedata_size(tox_));

    tox_get_savedata(tox_, reinterpret_cast<uint8_t*>(data.data()));

    std::ofstream s{config_.save_file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc};
    s.write(data.data(), static_cast<std::streamoff>(data.size()));

    TOXFS_LOG_INFO("Successfully saved tox savedata to {}", config_.save_file.native());
}

void impl_t::report_file_err_(unique_file_id_t id, tox_error error)
{
    recv_queue_ref_.push(recv_msg_file_error_t{id, std::move(error)});
}

void impl_t::check_chunk_requests_(std::optional<unique_file_id_t> opt_id)
{
    if (opt_id)
    {
        auto& file_id = *opt_id;
        auto& [num_in_flight, max_in_flight, last_update, requests] = chunk_requests_[file_id];

        while (num_in_flight < max_in_flight && !requests.empty())
        {
            recv_queue_ref_.push(recv_msg_file_chunk_request_t{ file_id, requests.front() });
            requests.pop();
            num_in_flight++;
        }
    }
    else
    {
        auto now = std::chrono::steady_clock::now();
        std::vector<unique_file_id_t> to_erase;
        for (auto& [file_id, req] : chunk_requests_)
        {
            auto& [num_in_flight, max_in_flight, last_update, requests] = req;

            if (now - last_update > std::chrono::seconds{60})
            {
                to_erase.push_back(file_id);
                continue;
            }

            if (num_in_flight == 0 && !requests.empty() && max_in_flight < 64)
            {
                max_in_flight *= 2;
            }

            while (num_in_flight < max_in_flight && !requests.empty())
            {
                recv_queue_ref_.push(recv_msg_file_chunk_request_t{ file_id, requests.front() });
                requests.pop();
                num_in_flight++;
            }
        }

        for (auto const& file_id : to_erase)
        {
            chunk_requests_.erase(file_id);
        }
    }
}

tox_t::tox_t(tox_config_t const& config)
    : impl_(std::make_unique<impl_t>(config))
{}

tox_t::~tox_t() noexcept
{}

std::shared_ptr<tox_if> tox_t::get_interface()
{
    return impl_->if_impl_ptr_;
}

void tox_t::start()
{
    impl_->loop_thread_ = std::thread([this]() { impl_->loop(); });
}

void tox_t::stop()
{
    impl_->loop_thread_.join();
}

void tox_t::save()
{
    impl_->send_queue_ref_.push(send_msg_savedata_t{});
}

} // namespace toxfs

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
#include "toxfs/util/scope_guard.hh"
#include "toxfs/util/message_queue.hh"
#include "toxfs/util/compile_utils.hh"
#include "toxfs_priv/tox/tox_msg_types.hh"

#include <tox/tox.h>

#include <fmt/ranges.h>

#include <string_view>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <thread>
#include <cassert>
#include <filesystem>
#include <optional>
#include <fstream>

// TODO: Remove this
namespace fs = std::filesystem;

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

void log_callback(Tox *, TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func,
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
}  // namespace detail

struct tox_t::impl_t
{
    message_queue<int, 16> mq_{};
    Tox *tox_ = nullptr;

    tox_config_t config_;
    std::optional<uint32_t> HACK_friend_number{};
    std::optional<uint32_t> HACK_file_number{};
    std::optional<std::ifstream> HACK_file{};
    std::vector<uint8_t> HACK_buffer{};

    impl_t(tox_config_t const& config);
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

    /**
     * Helper for binding a tox callback by passing this of impl_t as user_data
     * and then cast it back and call the corresponding member function after.
     */
    template <typename T> struct callback_t;
    template <typename ...Args>
    struct callback_t<void(tox_t::impl_t::*)(Args...)>
    {
        static const size_t nargs = sizeof...(Args);

        template <size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        };

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
{
    TOX_ERR_OPTIONS_NEW options_err;
    Tox_Options *options = nullptr;
    options = tox_options_new(&options_err);
    if (!options)
        throw TOXFS_EXCEPTION(tox::tox_error, "tox_options_new failed", options_err);

    scope_guard guard([options]() {
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

    address_t addr;
    tox_self_get_address(tox_, reinterpret_cast<uint8_t*>(&addr));

    TOXFS_LOG_INFO("My tox address is: {:02x}", fmt::join(addr.bytes, ""));

    TOXFS_LOG_DEBUG("Tox address breakdown: public_key: {:02x} nospam: {:02x} checksum: {:02x}",
        fmt::join(addr.public_key(), ""),
        fmt::join(addr.nospam(), ""),
        fmt::join(addr.checksum(), ""));

    while (true)
    {
        auto start = std::chrono::steady_clock::now();
        tox_iterate(tox_, this);
        std::this_thread::sleep_until(start + std::chrono::milliseconds{tox_iteration_interval(tox_)});
    }
}

void impl_t::on_friend_request(const uint8_t *public_key, const uint8_t *msg, size_t msg_len)
{
    const public_key_t& public_key_arr = *reinterpret_cast<const public_key_t*>(public_key);
    std::string_view msg_str{reinterpret_cast<const char*>(msg), msg_len};
    TOXFS_LOG_DEBUG("on_friend_request from {:x}: {} (len = {})", fmt::join(public_key_arr, ""), msg_str, msg_len);

    if (public_key_arr == config_.HACK_friend_address.public_key())
    {
        TOXFS_LOG_INFO("Accepting friend request!");
        TOX_ERR_FRIEND_ADD friend_add_error;
        uint32_t fr_num = tox_friend_add_norequest(tox_, public_key, &friend_add_error);
        if (friend_add_error)
        {
            TOXFS_LOG_ERROR("Error adding friend from request: {}", friend_add_error);
            return;
        }

        TOXFS_LOG_INFO("Added friend as number: {}", fr_num);
        HACK_friend_number = fr_num;
    }
    else
    {
        TOXFS_LOG_WARNING("Friend request does not match expected key, ignoring");
    }
}

void impl_t::on_friend_msg(uint32_t fr_num, TOX_MESSAGE_TYPE type, const uint8_t *msg, size_t msg_len)
{
    std::string_view msg_str{reinterpret_cast<const char*>(msg), msg_len};
    TOXFS_LOG_DEBUG("on_friend_msg from #{}: (type {}) {} (len = {})", fr_num, int(type), msg_str, msg_len);

    if (msg_str.size() >= 10 && msg_str.substr(0, 10) == "toxfs-send")
    {
        auto filename_start = msg_str.find_first_not_of(" \t", 10);
        if (filename_start == std::string_view::npos)
        {
            TOXFS_LOG_WARNING("Send command did not specify file!");
            return;
        }

        auto file = fs::path(msg_str.substr(filename_start));

        if (file.is_relative())
            file = config_.root_dir / file;

        if (!fs::exists(file))
        {
            TOXFS_LOG_ERROR("{} does not exist!", file.c_str());
            return;
        }

        file = fs::canonical(file);

        TOXFS_LOG_INFO("Request to send file: {}", file.c_str());

        std::error_code ec;
        auto rel_filename = fs::relative(file, config_.root_dir, ec);
        if (ec || *rel_filename.begin() == "..")
        {
            TOXFS_LOG_ERROR("file to send is not in root dir: {} (root = {}", file.c_str(), config_.root_dir.c_str());
            return;
        }

        assert(HACK_friend_number);

        HACK_file = std::ifstream{file, std::ios_base::in | std::ios_base::binary};

        TOX_ERR_FILE_SEND error;
        auto name = file.filename().generic_string();
        auto file_num = tox_file_send(tox_, *HACK_friend_number, TOX_FILE_KIND_DATA,
            fs::file_size(file), nullptr,
            reinterpret_cast<const uint8_t*>(name.c_str()), name.size(), &error);
        if (error)
        {
            TOXFS_LOG_ERROR("Failed to initiate file send: {}", error);
            return;
        }
        HACK_file_number = file_num;
    }
    else
    {
        TOXFS_LOG_INFO("Received non-command message: {}", msg_str);
    }
}

void impl_t::on_friend_name(uint32_t fr_num, const uint8_t *name, size_t name_len)
{
    std::string_view name_str{reinterpret_cast<const char*>(name), name_len};
    TOXFS_LOG_DEBUG("on_friend_msg from #{}: {} (len = {})", fr_num, name_str, name_len);
}

void impl_t::on_friend_status(uint32_t fr_num, const uint8_t *msg, size_t msg_len)
{
    std::string_view msg_str{reinterpret_cast<const char*>(msg), msg_len};
    TOXFS_LOG_DEBUG("on_friend_status from #{}: {} (len = {})", fr_num, msg_str, msg_len);
}

void impl_t::on_friend_conn_status(uint32_t fr_num, TOX_CONNECTION conn_status)
{
    TOXFS_LOG_DEBUG("on_friend_conn_status from #{}: {})", fr_num, int(conn_status));
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
}

void impl_t::on_file_recv(uint32_t fr_num, uint32_t file_num, uint32_t kind, uint64_t file_size,
    const uint8_t *filename, size_t filename_len)
{
    std::string_view filename_str{reinterpret_cast<const char*>(filename), filename_len};
    TOXFS_LOG_DEBUG("on_file_recv from #{}: file #{} {} size {} kind {}", fr_num, file_num,
        filename_str, file_size, kind);
}

void impl_t::on_file_chunk_request(uint32_t fr_num, uint32_t file_num, uint64_t position, size_t length)
{
    TOXFS_LOG_DEBUG("on_file_chunk_request from #{}: file #{} position {} len {}", fr_num, file_num, position, length);

    if (file_num != HACK_file_number)
    {
        TOXFS_LOG_ERROR("Chunk request for different file: {}", file_num);
        return;
    }

    if (!HACK_file)
    {
        TOXFS_LOG_ERROR("Chunk request with no valid file!");
        return;
    }

    if (length == 0)
    {
        TOXFS_LOG_INFO("Done Transferring (Probably...)");
        HACK_buffer.clear();
        HACK_file_number = std::nullopt;
        HACK_file = std::nullopt;
        return;
    }

    auto& stream = *HACK_file;
    stream.seekg(position);

    HACK_buffer.clear();
    HACK_buffer.resize(length);
    stream.read(reinterpret_cast<char*>(HACK_buffer.data()), length);

    TOX_ERR_FILE_SEND_CHUNK error;
    bool success = tox_file_send_chunk(tox_, fr_num, file_num, position, HACK_buffer.data(), length, &error);
    if (!success)
    {
        TOXFS_LOG_ERROR("Failed to send chunk ({}, {}): {}", position, length, error);
        return;
    }
}

void impl_t::on_file_chunk(uint32_t fr_num, uint32_t file_num, uint64_t position, const uint8_t *data, size_t data_len)
{
    TOXFS_LOG_DEBUG("on_file_chunk from #{}: file #{} position {} len {}", fr_num, file_num, position, data_len);
    (void)data;
}

tox_t::tox_t(tox_config_t const& config)
    : m_pImpl(std::make_unique<impl_t>(config))
{}

tox_t::~tox_t() noexcept
{}

std::shared_ptr<tox_if> tox_t::get_interface()
{
    return {};
}

void tox_t::start()
{
    m_pImpl->loop();
}

void tox_t::stop()
{
}

} // namespace toxfs

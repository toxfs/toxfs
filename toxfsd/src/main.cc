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

#include "toxfs/version.hh"
#include "toxfs/tox/tox.hh"
#include "toxfs/tox/tox_error.hh"
#include "toxfs/logging.hh"
#include "toxfs/exception.hh"
#include "toxfs/util/string_helpers.hh"
#include "toxfs/transfer/transfer_ctrl.hh"

#include <fmt/core.h>

#include <algorithm>

class friend_acceptor : public toxfs::tox::friend_callback_if
{
public:
    friend_acceptor(toxfs::tox::public_key_t fr_key)
        : fr_key_(fr_key)
    {}

    bool on_friend_request(toxfs::tox::public_key_t const& key) noexcept override
    {
        return key == fr_key_;
    }

private:
    toxfs::tox::public_key_t fr_key_;
};

int main(int argc, char **argv)
{
    {
        auto [major, minor, patch, hash] = toxfs::get_version();
        TOXFS_LOG_INFO("version: {}.{}.{} {}", major, minor, patch, hash);
    }
    {
        auto [major, minor, patch, hash] = toxfs::get_toxcore_version();
        TOXFS_LOG_INFO("toxcore version: {}.{}.{}", major, minor, patch);
    }

    if (argc <= 2)
    {
        TOXFS_LOG_WARNING("Missing arguments, cannot start!");
        TOXFS_LOG_WARNING("Usage: toxfsd <friend address> <root dir> [<savedata file>]");
        return 1;
    }

    std::string_view addr_hex(argv[1]);

    if (addr_hex.size() != toxfs::tox::k_address_size * 2)
    {
        TOXFS_LOG_ERROR("Address is unexpected length: expected {} characters, but got {}",
            sizeof(toxfs::tox::address_t) * 2, addr_hex.size());
        return 1;
    }

    toxfs::tox::tox_config_t config;
    auto friend_addr_bytes = toxfs::hex_string_to_binary(argv[1]);
    toxfs::tox::address_t friend_addr;
    std::copy_n(friend_addr_bytes.begin(), toxfs::tox::k_address_size, friend_addr.bytes.begin());

    config.root_dir = std::filesystem::canonical(argv[2]);
    TOXFS_LOG_INFO("Serving files from directory: {}", config.root_dir.c_str());

    if (argc >= 3)
    {
        config.save_file = std::filesystem::path{argv[3]};
    }

#ifdef NDEBUG
    toxfs::set_log_level(toxfs::log_level_t::info);
#endif

    auto tox = std::make_shared<toxfs::tox::tox_t>(config);
    friend_acceptor fr_acceptor{friend_addr.public_key()};
    tox->get_interface()->register_friend_callback_if(fr_acceptor);
    toxfs::transfer::transfer_ctrl tctrl{tox->get_interface(), config.root_dir};
    TOXFS_LOG_INFO("tox has initialized!");
    tox->start();

    try
    {
        auto tox_if = tox->get_interface();
        while (true)
        {
            auto [fr_id, message] = tox_if->get_message();
            if (message.size() >= 4 && message.substr(0, 4) == "send")
            {
                auto filename_start = message.find_first_not_of(" \t", 4);
                if (filename_start == std::string_view::npos)
                {
                    tox_if->send_message(fr_id, "send requires a argument").get();
                    continue;
                }

                try
                {
                    auto path = message.substr(filename_start);
                    TOXFS_LOG_DEBUG("send_path Fr#{}: {}", fr_id.id, path);
                    tctrl.send_path(fr_id, path);
                    TOXFS_LOG_DEBUG("done send_path Fr#{}: {}", fr_id.id, path);
                    tox_if->send_message(fr_id, fmt::format("done {}", message)).get();
                }
                catch (toxfs::exception const& e)
                {
                    tox_if->send_message(fr_id, fmt::format("error {}", e.what())).get();
                }
            }
            else if (message.size() >= 4 && message.substr(0, 4) == "save")
            {
                tox->save();
            }
            else
            {
                tox_if->send_message(fr_id, fmt::format("unknown command: {}", message)).get();
            }
        }
    }
    catch (toxfs::runtime_error const& e)
    {
        toxfs::runtime_error e2 = e;
        TOXFS_LOG_ERROR("tox failed: {}, {}:{}", e2.what(), e2.file(), e2.line());
    }
    tox->stop();
    return 0;
}

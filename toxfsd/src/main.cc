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

#include <fmt/core.h>

#include <string_view>
#include <charconv>

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
        TOXFS_LOG_WARNING("Usage: toxfsd <friend address> <root dir>");
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
    for (size_t i = 0; i < toxfs::tox::k_address_size; ++i)
    {
        uint8_t b = 0;
        auto [ptr, ec] = std::from_chars(&addr_hex[2 * i], &addr_hex[2 * i + 2], b, 16);
        if (ec != std::errc())
        {
            TOXFS_LOG_ERROR("Address contains non-hex characters: {}{}", addr_hex[i], addr_hex[i + 1]);
            return 1;
        }

        config.HACK_friend_address.bytes[i] = std::byte{b};
    }

    config.root_dir = std::filesystem::canonical(argv[2]);
    TOXFS_LOG_INFO("Serving files from directory: {}", config.root_dir.c_str());

#ifdef NDEBUG
    toxfs::set_log_level(toxfs::log_level_t::info);
#endif

    try
    {
        auto tox = std::make_shared<toxfs::tox::tox_t>(config);
        TOXFS_LOG_INFO("tox has initialized!");
        tox->start();
    }
    catch (toxfs::runtime_error const& e)
    {
        toxfs::runtime_error e2 = e;
        TOXFS_LOG_ERROR("tox failed: {}, {}:{}", e2.what(), e2.file(), e2.line());
    }
    return 0;
}

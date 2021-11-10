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

#include "toxfs/util/string_helpers.hh"
#include "toxfs/exception.hh"

#include <fmt/core.h>

#include <charconv>

namespace toxfs
{

std::vector<std::byte> hex_string_to_binary(std::string_view s)
{
    if (s.size() % 2u != 0u)
    {
        throw TOXFS_EXCEPTION(runtime_error, fmt::format("String has non-even number of characters: {}", s.size()));
    }

    std::vector<std::byte> ret;
    ret.reserve(s.size() / 2);

    for (size_t i = 0; i < s.size() / 2; ++i)
    {
        uint8_t b = 0;
        auto [ptr, ec] = std::from_chars(&s[2 * i], &s[2 * i + 2], b, 16);
        if (ec != std::errc())
        {
            throw TOXFS_EXCEPTION(runtime_error, fmt::format("String has non-hex characters: {}", s.substr(i, 2)));
        }
        ret.push_back(std::byte{b});
    }

    return ret;
}

} // namespace toxfs

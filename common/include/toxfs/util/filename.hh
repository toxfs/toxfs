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

#pragma once

#include <cstddef>

namespace toxfs::detail
{

template<std::size_t N>
constexpr const char * file_to_filename(const char (&arr)[N], size_t count)
{
    size_t i = N - 1;
    for (; i > 0; --i)
    {
        if (arr[i] == '/' || arr[i] == '\\')
            count -= 1;
        if (count == 0)
            return &arr[i + 1];
    }
    return &arr[0];
}

} // namespace toxfs::detail

#define TOXFS_FILENAME ::toxfs::detail::file_to_filename(__FILE__, 2)

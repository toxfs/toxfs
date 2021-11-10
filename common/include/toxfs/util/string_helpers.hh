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

#include <fmt/core.h>

#include <string_view>
#include <vector>
#include <cstddef>

namespace toxfs
{

/**
 * @brief convert a hex string to binary representation
 * @param s - the hex string
 * @return the hex string's binary equivalent
 * @throws if s.size() is not a multiple of 2 or s contains non-hex characters
 */
std::vector<std::byte> hex_string_to_binary(std::string_view s);

} // namespace toxfs

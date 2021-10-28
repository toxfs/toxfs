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

#include <cstdint>
#include <cstddef>
#include <array>

namespace toxfs::tox
{

constexpr size_t k_public_key_size = 32;

using public_key_t = std::array<std::byte, k_public_key_size>;
enum class no_spam_t : uint32_t {};
enum class checksum_t : uint16_t {};

struct address_t
{
    public_key_t public_key;
    no_spam_t nospam;
    checksum_t checksum;
};

enum class connection_t
{
    none,
    tcp,
    udp
};

} // namespace toxfs::tox

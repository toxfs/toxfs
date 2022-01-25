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
#include <string>

namespace toxfs::tox
{

constexpr size_t k_public_key_size = 32;
constexpr size_t k_nospam_size = 4;
constexpr size_t k_checksum_size = 2;
constexpr size_t k_address_size = k_public_key_size + k_nospam_size + k_checksum_size;

using public_key_t = std::array<std::byte, k_public_key_size>;
using nospam_t = std::array<std::byte, k_nospam_size>;
using checksum_t = std::array<std::byte, k_checksum_size>;

struct address_t
{
private:
    template<size_t Start, size_t Size>
    constexpr std::array<std::byte, Size> subarr_() const noexcept
    {
        std::array<std::byte, Size> ret;
        for (size_t i = Start; i < Start + Size; ++i)
            ret[i - Start] = bytes[i];
        return ret;
    }

public:
    std::array<std::byte, k_address_size> bytes;

    constexpr public_key_t public_key() const noexcept
    {
        return subarr_<0, k_public_key_size>();
    }

    constexpr nospam_t nospam() const noexcept
    {
        return subarr_<k_public_key_size, k_nospam_size>();
    }

    constexpr checksum_t checksum() const noexcept
    {
        return subarr_<k_public_key_size + k_nospam_size, k_checksum_size>();
    }
};

enum class connection_t
{
    none,
    tcp,
    udp
};

struct friend_id_t
{
    uint32_t id;

    constexpr bool operator==(toxfs::tox::friend_id_t const& o) const noexcept
    {
        return id == o.id;
    }
};

struct message_id_t
{
    uint32_t id;

    constexpr bool operator==(toxfs::tox::message_id_t const& o) const noexcept
    {
        return id == o.id;
    }
};

struct friend_message_t
{
    friend_id_t id;
    std::string message;
};

} // namespace toxfs::tox


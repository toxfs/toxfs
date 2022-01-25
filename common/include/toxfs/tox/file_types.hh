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

#include "toxfs/tox/tox_types.hh"
#include "toxfs/tox/tox_error.hh"

#include "toxfs/util/buffer.hh"

#include <cstdint>
#include <functional>

namespace toxfs::tox
{

struct file_id_t
{
    uint32_t id;

    constexpr bool operator==(toxfs::tox::file_id_t const& o) const noexcept
    {
        return id == o.id;
    }
};

struct unique_file_id_t
{
    friend_id_t friend_id;
    file_id_t file_id;

    constexpr bool operator==(toxfs::tox::unique_file_id_t const& o) const noexcept
    {
        return friend_id == o.friend_id && file_id == o.file_id;
    }
};

enum class file_control_t
{
    resume,
    pause,
    cancel
};

struct file_info_t
{
    std::string filename;
    uint64_t filesize;
};

struct file_chunk_request_t
{
    uint64_t position;
    size_t size;
};

struct file_chunk_t
{
    uint64_t position;
    buffer_t data;
};

} // namespace toxfs::tox

template<>
struct std::hash<toxfs::tox::unique_file_id_t>
{
    std::size_t operator()(toxfs::tox::unique_file_id_t const& n) const noexcept
    {
        uint64_t num = static_cast<uint64_t>(n.friend_id.id) << 32u | n.file_id.id;
        return std::hash<uint64_t>{}(num);
    }
};

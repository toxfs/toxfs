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

#include "toxfs/util/chunked_progress.hh"
#include "toxfs/logging.hh"

namespace toxfs
{

chunked_progress::chunked_progress(uint64_t size)
    : size_(size)
    , pos_(0u)
{
}

void chunked_progress::update(uint64_t pos, uint64_t size) noexcept
{
    if (pos <= pos_ && pos + size > pos_)
    {
        pos_ = pos + size;
    }
    else if (pos > pos_)
    {
        TOXFS_LOG_DEBUG("Progress jumped from {} to {}", pos_, pos);
        pos_ = pos + size;
    }
}

uint64_t chunked_progress::progress() const noexcept
{
    return pos_;
}

uint64_t chunked_progress::total_size() const noexcept
{
    return size_;
}

} // namespace toxfs

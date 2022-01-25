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

#include <cstdint>
#include <array>

namespace toxfs
{

/**
 * A class that tracks the read/write progress of a finite sized contiguous piece of
 * data (usually a file). To a certain extent it can handle operations happening out of
 * linear order.
 */
class chunked_progress
{
public:
    /**
     * @brief ctor
     * @param[in] size - the total size of the data.
     * @throws if size exceeds the maximum trackable size
     */
    explicit chunked_progress(uint64_t size);

    chunked_progress(chunked_progress&& other) = default;
    chunked_progress& operator=(chunked_progress&& other) = default;

    chunked_progress(chunked_progress const& other) = default;
    chunked_progress& operator=(chunked_progress const& other) = default;

    /**
     * @brief update the progress with a new read/write
     * @param[in] pos - the position
     * @param[in] size - the length
     */
    void update(uint64_t pos, uint64_t size) noexcept;

    /**
     * @brief returns the total
     */
    uint64_t progress() const noexcept;

    /**
     * @brief returns the total
     */
    uint64_t total_size() const noexcept;

private:
    uint64_t size_;
    uint64_t pos_;
    /* TODO: Implement out of order handling */
    //std::array<int64_t, 7> progress_arr_;
};

};

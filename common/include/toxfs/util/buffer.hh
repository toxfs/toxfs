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

#include <memory>

namespace toxfs
{

/**
 * A helper for managing an dynamically allocated array with a fixed
 * capacity.
 */
class buffer_t
{
public:
    /**
     * @brief ctor
     * @param[in] capacity - the capacity of the buffer
     */
    explicit buffer_t(size_t capacity)
        // TODO: make_shared is more efficient but only works with arrays in C++20
        : buffer_(capacity > 0 ? new std::byte[capacity] : nullptr)
        , size_(0)
        , capacity_(capacity)
    {}

    ~buffer_t() noexcept = default;

    buffer_t(buffer_t const&) = default;
    buffer_t& operator=(buffer_t const&) = default;

    buffer_t(buffer_t&&) = default;
    buffer_t& operator=(buffer_t&&) = default;

    /**
     * @brief get the data buffer
     * @return the pointer to the buffer
     */
    std::byte* data() noexcept { return buffer_.get(); }

    std::byte const* data() const noexcept { return buffer_.get(); }

    size_t size() const noexcept { return size_; }

    size_t capacity() const noexcept { return capacity_; }

    size_t set_size(size_t s) noexcept { return size_ = s; }

private:
    std::shared_ptr<std::byte[]> buffer_;
    size_t size_;
    size_t capacity_;
};

} // namespace toxfs

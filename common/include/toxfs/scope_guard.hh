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

#include <utility>

namespace toxfs
{

template<class F>
class scope_guard
{
public:

    /**
     * @brief ctor
     * @param[in] callable - function to call
     */
    explicit scope_guard(F&& callable) noexcept
        : callable_(std::forward<F>(callable))
    {
    }

    /**
     * @brief move ctor
     * @param[in] other - the moved
     */
    scope_guard(scope_guard && other) noexcept
        : callable_(std::move(other.callable_))
    {
    }

    scope_guard& operator=(scope_guard &&) noexcept = delete;

    scope_guard& operator=(scope_guard const&) = delete;
    scope_guard(scope_guard const&) = delete;

    /**
     * @brief prevent the scope guard from running
     */
    void release() noexcept
    {
        enabled_ = false;
    }

    ~scope_guard() noexcept
    {
        if (enabled_)
        {
            callable_();
        }
    }

private:
    F callable_;
    bool enabled_ = true;
};

} // namespace toxfs

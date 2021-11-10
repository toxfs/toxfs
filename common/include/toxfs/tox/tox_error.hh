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

#include "toxfs/exception.hh"

#include <type_traits>

namespace toxfs::tox
{

class tox_error : public toxfs::runtime_error
{
public:
    template <class Enum, std::enable_if_t<std::is_enum<Enum>::value, bool> = true>
    explicit tox_error(const char* what, const char *file, int line, Enum errc)
        : toxfs::runtime_error(make_err_msg_(what, static_cast<int>(errc)), file, line)
        , errc_(static_cast<int>(errc))
    {}

    template <class Enum, std::enable_if_t<std::is_enum<Enum>::value, bool> = true>
    explicit tox_error(std::string const& what, const char *file, int line, Enum errc)
        : toxfs::runtime_error(make_err_msg_(what, static_cast<int>(errc)), file, line)
        , errc_(static_cast<int>(errc))
    {}

    ~tox_error() noexcept override = default;

    tox_error(tox_error const&) noexcept = default;
    tox_error& operator=(tox_error const&) noexcept = default;

    /**
     * @brief Get the error code
     */
    int error_code() const noexcept
    {
        return errc_;
    }

private:
    static std::string make_err_msg_(const char* const& what, int errc);
    static std::string make_err_msg_(std::string const& what, int errc);

    int errc_;
};

} // namespace toxfs::tox

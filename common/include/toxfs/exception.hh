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

#include "toxfs/util/filename.hh"

#include <stdexcept>
#include <string>

namespace toxfs
{

/**
 * Abstract base class of toxfs exceptions
 */
class exception : public std::exception
{
public:
    /**
     * @brief Get the file the exception was thrown from
     * @return null terminated string of filename, empty if none
     */
    virtual const char* file() const noexcept = 0;

    /**
     * @brief Get the line number the exception in file
     * @return the line number, 0 if none
     */
    virtual int line() const noexcept = 0;
};

template <typename E>
class exception_tmpl : public E, public toxfs::exception
{
public:
    explicit exception_tmpl(const char* what, const char *file = "", int line = 0) noexcept
        : E(what)
        , file_(file)
        , line_(line)
    {}

    explicit exception_tmpl(std::string const& what, const char *file = "", int line = 0) noexcept
        : E(what)
        , file_(file)
        , line_(line)
    {}

    ~exception_tmpl() noexcept override = default;

    exception_tmpl(exception_tmpl const&) noexcept = default;
    exception_tmpl& operator=(exception_tmpl const&) noexcept = default;

    const char* what() const noexcept override
    {
        return E::what();
    }

    const char* file() const noexcept override
    {
        return file_;
    }

    int line() const noexcept override
    {
        return line_;
    }

private:
    const char *file_;
    int line_;
};

using logic_error = exception_tmpl<std::logic_error>;
using runtime_error = exception_tmpl<std::runtime_error>;

} // namespace toxfs

/**
 * Throws an toxfs::<e_type> with location information
 */
#define TOXFS_EXCEPTION(e_type_, what_str_, ...) ::toxfs::e_type_{what_str_, TOXFS_FILENAME, __LINE__, ##__VA_ARGS__}

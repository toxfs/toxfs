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

#include "toxfs/tox/tox_error.hh"

#include <fmt/core.h>

namespace toxfs::tox
{
    /*static*/ std::string tox_error::make_err_msg_(const char* const& what, int errc)
    {
        return fmt::format("{}, errc = {}", what, errc);
    }

    /*static*/ std::string tox_error::make_err_msg_(std::string const& what, int errc)
    {
        return fmt::format("{}, errc = {}", what, errc);
    }
} // namespace toxfs::tox

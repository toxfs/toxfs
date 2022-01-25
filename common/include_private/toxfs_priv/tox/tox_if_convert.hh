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
#include "toxfs/tox/file_types.hh"
#include "toxfs/exception.hh"

#include <tox/tox.h>

namespace toxfs::tox
{

namespace from_tox
{

inline file_control_t convert(TOX_FILE_CONTROL tox_type)
{
    switch (tox_type)
    {
        case TOX_FILE_CONTROL_CANCEL: return file_control_t::cancel;
        case TOX_FILE_CONTROL_PAUSE: return file_control_t::pause;
        case TOX_FILE_CONTROL_RESUME: return file_control_t::resume;
    };

    throw TOXFS_EXCEPTION(logic_error, "Invalid TOX_FILE_CONTROL");
}

} // namespace from_tox

namespace to_tox
{

inline TOX_FILE_CONTROL convert(file_control_t tox_type)
{
    switch (tox_type)
    {
        case file_control_t::cancel: return TOX_FILE_CONTROL_CANCEL;
        case file_control_t::pause: return TOX_FILE_CONTROL_PAUSE;
        case file_control_t::resume: return TOX_FILE_CONTROL_RESUME;
    };

    throw TOXFS_EXCEPTION(logic_error, "Invalid file_control_t");
}

}

} // namespace toxfs::tox


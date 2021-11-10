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

#include "toxfs/version.hh"
#include "toxfs_priv/cmake_version.hh"

#include <tox/tox.h>

namespace toxfs
{

version_t get_version()
{
    version_t v;
    v.major = TOXFS_VERSION_MAJOR;
    v.minor = TOXFS_VERSION_MINOR;
    v.patch = TOXFS_VERSION_PATCH;
#ifdef TOXFS_GIT_HASH
    v.git_hash = TOXFS_GIT_HASH;
#else
    v.git_hash = "unknown";
#endif
    return v;
}

version_t get_toxcore_version()
{
    version_t v;
    v.major = tox_version_major();
    v.minor = tox_version_minor();
    v.patch = tox_version_patch();
    v.git_hash = "";
    return v;
}

} // namespace Toxfs

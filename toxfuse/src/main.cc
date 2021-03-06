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
#include "toxfs/logging.hh"

#include <fmt/core.h>

int main()
{
    {
        auto [major, minor, patch, hash] = toxfs::get_version();
        TOXFS_LOG_INFO("version: {}.{}.{} {}\n", major, minor, patch, hash);
    }
    {
        auto [major, minor, patch, hash] = toxfs::get_toxcore_version();
        TOXFS_LOG_INFO("toxcore version: {}.{}.{} {}\n", major, minor, patch, hash);
    }
    return 0;
}

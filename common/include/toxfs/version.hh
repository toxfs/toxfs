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

/**
 * Version Information
 */

#include <cstdint>

namespace toxfs
{

struct version_t
{
    /* Major Number */
    uint32_t major = 0;
    /* Minor Number */
    uint32_t minor = 0;
    /* Patch Number */
    uint32_t patch = 0;
    /* Git Hash String */
    const char *git_hash = "";
};

/**
 * @brief Get the version
 * @return toxfs version
 */
version_t get_version();


/**
 * @brief Get the toxcore version
 * @return toxcore version
 */
version_t get_toxcore_version();

} // namespace Toxfs

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
 * Tox Interface
 */

#include <toxfs/tox/tox_types.hh>
#include <toxfs/tox/tox_if.hh>

#include <memory>

namespace toxfs::tox
{

struct config_t
{
    address_t local_address;

};

class tox_t : public std::enable_shared_from_this<tox_t>
{
    tox_t();

    ~tox_t();

    /**
     * @brief Get tox_if
     */
    std::shared_ptr<tox_if> get_interface();

    void start();

    void stop();

private:
    void loop();

    struct impl_t;
    std::unique_ptr<impl_t> m_pImpl;
};

} // namespace toxfs::tox
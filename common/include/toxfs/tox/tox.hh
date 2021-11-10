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

#include "toxfs/tox/tox_types.hh"
#include "toxfs/tox/tox_if.hh"

#include <memory>
#include <filesystem>

namespace toxfs::tox
{

struct tox_config_t
{
    address_t local_address{};
    address_t HACK_friend_address{};
    std::filesystem::path root_dir{};
};

class tox_t : public std::enable_shared_from_this<tox_t>
{
public:
    explicit tox_t(tox_config_t const& config);

    ~tox_t() noexcept;

    /**
     * @brief Get tox_if
     */
    std::shared_ptr<tox_if> get_interface();

    void start();

    void stop();

    struct impl_t;
private:
    std::unique_ptr<impl_t> m_pImpl;
};

} // namespace toxfs::tox

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

#include <toxfs/tox.hh>
#include <toxfs/scope_guard.hh>
#include <toxfs/message_queue.hh>

#include <tox/tox.h>

#include <stdexcept>
#include <mutex>

namespace toxfs
{

namespace detail
{
namespace
{

}
}  // namespace detail

struct tox_t::impl_t
{
    message_queue<int, 4> mq_{};
    Tox *tox_ = nullptr;
};

tox_t::tox_t()
    : m_pImpl(std::make_unique<impl_t>())
{
    // TODO: better exceptions
    Tox_Options *options = nullptr;
    options = tox_options_new(nullptr);
    if (!options)
        throw std::runtime_error("Failed to allocate Tox_Options");

    scope_guard guard([options]() {
        tox_options_free(options);
    });

    m_pImpl->tox_ = tox_new(options, nullptr);
    if (!m_pImpl->tox_)
        throw std::runtime_error("Failed to allocate Tox");
}

tox_t::~tox_t()
{
    if (m_pImpl->tox_)
    {
        tox_kill(m_pImpl->tox_);
    }
}

} // namespace toxfs

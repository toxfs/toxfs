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

#include <future>

namespace toxfs::tox
{

class friend_callback_if
{
public:
    virtual ~friend_callback_if() noexcept = default;

    /**
     * @brief callback for a friend request
     * @return TODO: returns true to accept
     */
    virtual bool on_friend_request(public_key_t const& key) noexcept = 0;

};

class file_callback_if
{
public:
    virtual ~file_callback_if() noexcept = default;

    /**
     * @brief callback for file control requests
     * @param[in] id - the file id
     * @param[in] info - the information on the file
     */
    virtual void on_tox_file_recv(unique_file_id_t id, file_info_t info) noexcept = 0;

    /**
     * @brief callback for file control requests
     * @param[in] id - the file id
     * @param[in] control - the control command
     */
    virtual void on_tox_file_control(unique_file_id_t id, file_control_t control) noexcept = 0;

    /**
     * @brief callback for file chunk requests
     * @param[in] id - the file id
     * @param[in] request - the chunk request
     */
    virtual void on_tox_file_chunk_request(unique_file_id_t id, file_chunk_request_t request) noexcept = 0;

    /**
     * @brief callback for file chunks received
     * @param[in] id - the file id
     * @param[in] chunk - the chunk
     */
    virtual void on_tox_file_chunk_receive(unique_file_id_t id, file_chunk_t chunk) noexcept = 0;

    /**
     * @brief callback for a file related error
     * @param[in] id - the file id
     * @param[in] err - the error
     */
    virtual void on_tox_file_error(unique_file_id_t id, tox_error err) noexcept = 0;
};

/**
 * Interface for tox
 */
class tox_if
{
public:
    virtual ~tox_if() noexcept = default;

    /**
     * @brief get the connection status of a friend
     * @return the status
     */
    virtual std::future<connection_t> get_connection_status() = 0;

    /* TEMPORARY */
    virtual friend_message_t get_message() = 0;

    virtual std::future<message_id_t> send_message(friend_id_t id, std::string message) = 0;

    /**
     * @brief register the friend callback if
     * @param[in] friend_callback_if - the friend callback if
     */
    virtual void register_friend_callback_if(friend_callback_if& friend_callback_if) = 0;

    /**
     * @brief unregister the friend callback if
     * @param[in] friend_callback_if - the friend callback if
     */
    virtual void unregister_friend_callback_if(friend_callback_if& friend_callback_if) = 0;

    /**
     * @brief register the file callback if
     * @param[in] file_if - the file callback if
     */
    virtual void register_file_callback_if(file_callback_if& file_if) = 0;

    /**
     * @brief unregister the file callback if
     * @param[in] file_if - the file callback if
     */
    virtual void unregister_file_callback_if(file_callback_if& file_if) = 0;

    /**
     * @brief send a file
     * @param[in] fr_id - friend to send to
     * @param[in] info - info on the file to send
     * @return the file id
     */
    virtual std::future<unique_file_id_t> send_file(friend_id_t fr_id, file_info_t file) = 0;

    /**
     * @brief send a file control
     * @param[in] id - the file id
     * @param[in] control - the file control
     */
    virtual void send_file_control(unique_file_id_t id, file_control_t control) = 0;

    /**
     * @brief send a file chunk
     * @param[in] id - the file id
     * @param[in] chunk - the file chunk
     */
    virtual void send_file_chunk(unique_file_id_t id, file_chunk_t chunk) = 0;
};


} // namespace toxfs::tox

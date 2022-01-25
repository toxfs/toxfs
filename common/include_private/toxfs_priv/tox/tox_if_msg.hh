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
#include "toxfs/tox/tox_error.hh"

#include <variant>
#include <future>
#include <cstdint>

namespace toxfs::tox
{
    struct recv_msg_fr_request_t
    {
        tox::public_key_t key;
        std::string message;
    };

    struct recv_msg_fr_message_t
    {
        friend_id_t id;
        std::string message;
    };

    struct recv_msg_fr_name_t
    {
        friend_id_t id;
        std::string name;
    };

    struct recv_msg_fr_status_t
    {
        friend_id_t id;
        std::string status;
    };

    struct recv_msg_file_receive_t
    {
        unique_file_id_t id;
        file_info_t info;
    };

    struct recv_msg_file_control_t
    {
        unique_file_id_t id;
        file_control_t control;
    };

    struct recv_msg_file_chunk_request_t
    {
        unique_file_id_t id;
        file_chunk_request_t chunk_request;
    };

    struct recv_msg_file_chunk_t
    {
        unique_file_id_t id;
        file_chunk_t chunk;
    };

    struct recv_msg_file_error_t
    {
        unique_file_id_t id;
        tox_error error;
    };

    using recv_msg_t = std::variant<
        recv_msg_fr_request_t,
        recv_msg_fr_message_t,
        recv_msg_fr_name_t,
        recv_msg_fr_status_t,
        recv_msg_file_receive_t,
        recv_msg_file_control_t,
        recv_msg_file_chunk_request_t,
        recv_msg_file_chunk_t,
        recv_msg_file_error_t
    >;

    struct send_msg_get_conn_status_t
    {
        std::promise<connection_t> promise;
    };

    struct send_msg_accept_fr_req_t
    {
        public_key_t public_key;
        std::promise<friend_id_t> promise;
    };

    struct send_msg_fr_message_t
    {
        friend_id_t id;
        std::string message;
        std::promise<message_id_t> promise;
    };

    struct send_msg_file_send_t
    {
        friend_id_t id;
        file_info_t info;
        std::promise<unique_file_id_t> promise;
    };

    struct send_msg_file_control_t
    {
        unique_file_id_t id;
        file_control_t control;
    };

    struct send_msg_file_chunk_t
    {
        unique_file_id_t id;
        file_chunk_t chunk;
    };

    struct send_msg_savedata_t
    {
    };

    using send_msg_t = std::variant<
        send_msg_get_conn_status_t,
        send_msg_accept_fr_req_t,
        send_msg_fr_message_t,
        send_msg_file_send_t,
        send_msg_file_control_t,
        send_msg_file_chunk_t,
        send_msg_savedata_t
    >;
} // namespace toxfs::tox

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

#include "toxfs/tox/tox_if.hh"
#include "toxfs/util/message_queue.hh"

#include "toxfs_priv/tox/tox_if_msg.hh"

#include <thread>

namespace toxfs::tox
{

constexpr size_t k_queue_max_size = 512;

using send_queue_t = message_queue<send_msg_t, k_queue_max_size>;
using recv_queue_t = message_queue<recv_msg_t, k_queue_max_size>;

class tox_if_impl : public tox_if
{
public:
    tox_if_impl();

    ~tox_if_impl() noexcept;

    /* tox_if */

    std::future<connection_t> get_connection_status() override;

    friend_message_t get_message() override;

    std::future<message_id_t> send_message(friend_id_t id, std::string message) override;

    void register_friend_callback_if(friend_callback_if& friend_if) override;

    void unregister_friend_callback_if(friend_callback_if& friend_if) override;

    void register_file_callback_if(file_callback_if& file_if) override;

    void unregister_file_callback_if(file_callback_if& file_if) override;

    std::future<unique_file_id_t> send_file(friend_id_t fr_id, file_info_t file) override;

    void send_file_control(unique_file_id_t id, file_control_t control) override;

    void send_file_chunk(unique_file_id_t id, file_chunk_t chunk) override;

    /* END tox_if */

    send_queue_t& get_send_queue() noexcept { return send_queue_; }

    recv_queue_t& get_recv_queue() noexcept { return recv_queue_; }

private:
    void recv_msg_(recv_msg_fr_request_t&& msg);
    void recv_msg_(recv_msg_fr_message_t&& msg);
    void recv_msg_(recv_msg_fr_name_t&& msg);
    void recv_msg_(recv_msg_fr_status_t&& msg);
    void recv_msg_(recv_msg_file_receive_t&& msg);
    void recv_msg_(recv_msg_file_control_t&& msg);
    void recv_msg_(recv_msg_file_chunk_request_t&& msg);
    void recv_msg_(recv_msg_file_chunk_t&& msg);
    void recv_msg_(recv_msg_file_error_t&& msg);

    void msg_thread_run_() noexcept;

    std::thread msg_thread_;
    send_queue_t send_queue_;
    recv_queue_t recv_queue_;
    friend_callback_if *friend_callback_if_ = nullptr;
    file_callback_if *file_callback_if_ptr_ = nullptr;

    message_queue<friend_message_t, 64> fr_messages_queue_;
};

} // namespace toxfs::tox

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

#include "toxfs/exception.hh"
#include "toxfs/logging.hh"

#include "toxfs_priv/tox/tox_if_impl.hh"

#include <cassert>

namespace toxfs::tox
{

tox_if_impl::tox_if_impl()
{
    msg_thread_ = std::thread([this]() { msg_thread_run_(); });
}

tox_if_impl::~tox_if_impl() noexcept
{
    msg_thread_.join();
}

std::future<connection_t> tox_if_impl::get_connection_status()
{
    send_msg_get_conn_status_t msg
    {
        std::promise<connection_t>{}
    };

    auto future = msg.promise.get_future();
    send_queue_.push(std::move(msg));
    return future;
}

friend_message_t tox_if_impl::get_message()
{
    return fr_messages_queue_.pop();
}

std::future<message_id_t> tox_if_impl::send_message(friend_id_t id, std::string message)
{
    send_msg_fr_message_t msg
    {
        id,
        std::move(message),
        std::promise<message_id_t>{}
    };

    auto future = msg.promise.get_future();
    send_queue_.push(std::move(msg));
    return future;
}

void tox_if_impl::register_friend_callback_if(friend_callback_if& friend_if)
{
    if (friend_callback_if_)
        throw TOXFS_EXCEPTION(runtime_error, "friend_callback_if already registered!");

    friend_callback_if_ = &friend_if;
}

void tox_if_impl::unregister_friend_callback_if(friend_callback_if& friend_if)
{
    if (!friend_callback_if_)
        throw TOXFS_EXCEPTION(runtime_error, "file_callback_if not registered!");

    if (friend_callback_if_ != &friend_if)
        throw TOXFS_EXCEPTION(runtime_error, "trying to unregister unrelated friend_callback!");

    friend_callback_if_ = nullptr;
}

void tox_if_impl::register_file_callback_if(file_callback_if& file_if)
{
    if (file_callback_if_ptr_)
        throw TOXFS_EXCEPTION(runtime_error, "file_callback_if already registered!");

    file_callback_if_ptr_ = &file_if;
}

void tox_if_impl::unregister_file_callback_if(file_callback_if& file_if)
{
    if (!file_callback_if_ptr_)
        throw TOXFS_EXCEPTION(runtime_error, "file_callback_if not registered!");

    if (file_callback_if_ptr_ != &file_if)
        throw TOXFS_EXCEPTION(runtime_error, "trying to unregister unrelated file_callback_if!");

    file_callback_if_ptr_ = nullptr;
}

std::future<unique_file_id_t> tox_if_impl::send_file(friend_id_t fr_id, file_info_t file)
{
    send_msg_file_send_t msg
    {
        fr_id,
        file,
        std::promise<unique_file_id_t>{}
    };

    auto future = msg.promise.get_future();
    send_queue_.push(std::move(msg));
    return future;
}

void tox_if_impl::send_file_control(unique_file_id_t id, file_control_t control)
{
    send_msg_file_control_t msg
    {
        id,
        control
    };

    send_queue_.push(std::move(msg));
}

void tox_if_impl::send_file_chunk(unique_file_id_t id, file_chunk_t chunk)
{
    send_msg_file_chunk_t msg
    {
        id,
        std::move(chunk)
    };

    send_queue_.push(std::move(msg));
}

void tox_if_impl::recv_msg_(recv_msg_fr_request_t&& msg)
{
    if (friend_callback_if_)
    {
        if (friend_callback_if_->on_friend_request(msg.key))
        {
            send_msg_accept_fr_req_t accept_msg
            {
                msg.key,
                std::promise<friend_id_t>{}
            };

            send_queue_.push(std::move(accept_msg));
            return;
        }
    }

    TOXFS_LOG_WARNING("Ignoring friend request from {:02x}", fmt::join(msg.key, ""));
}

void tox_if_impl::recv_msg_(recv_msg_fr_message_t&& msg)
{
    fr_messages_queue_.push(friend_message_t{msg.id, std::move(msg.message)});
}

void tox_if_impl::recv_msg_(recv_msg_fr_name_t&& msg)
{
    (void)msg;
}

void tox_if_impl::recv_msg_(recv_msg_fr_status_t&& msg)
{
    (void)msg;
}

void tox_if_impl::recv_msg_(recv_msg_file_receive_t&& msg)
{
    if (file_callback_if_ptr_)
    {
        file_callback_if_ptr_->on_tox_file_recv(msg.id, msg.info);
    }
    else
    {
        TOXFS_LOG_ERROR("Unhandled file recv message");
    }
}

void tox_if_impl::recv_msg_(recv_msg_file_control_t&& msg)
{
    if (file_callback_if_ptr_)
    {
        file_callback_if_ptr_->on_tox_file_control(msg.id, msg.control);
    }
    else
    {
        TOXFS_LOG_ERROR("Unhandled file control message");
    }
}

void tox_if_impl::recv_msg_(recv_msg_file_chunk_request_t&& msg)
{
    if (file_callback_if_ptr_)
    {
        file_callback_if_ptr_->on_tox_file_chunk_request(msg.id, msg.chunk_request);
    }
    else
    {
        TOXFS_LOG_ERROR("Unhandled file chunk request message");
    }
}

void tox_if_impl::recv_msg_(recv_msg_file_chunk_t&& msg)
{
    if (file_callback_if_ptr_)
    {
        file_callback_if_ptr_->on_tox_file_chunk_receive(msg.id, std::move(msg.chunk));
    }
    else
    {
        TOXFS_LOG_ERROR("Unhandled file chunk message");
    }
}

void tox_if_impl::recv_msg_(recv_msg_file_error_t&& msg)
{
    if (file_callback_if_ptr_)
    {
        file_callback_if_ptr_->on_tox_file_error(msg.id, std::move(msg.error));
    }
    else
    {
        TOXFS_LOG_ERROR("Unhandled file error message");
    }
}

void tox_if_impl::msg_thread_run_() noexcept
{
    // TODO: shutdown thread
    while (true)
    {
        auto opt_msg = recv_queue_.pop_timeout(std::chrono::milliseconds{100});
        if (opt_msg)
        {
            try
            {
                std::visit([this](auto&& msg) { recv_msg_(std::move(msg)); }, *opt_msg);
            }
            catch (toxfs::exception const& e)
            {
                TOXFS_LOG_ERROR("Exception while handling recevied message: {}", e.what());
            }
            catch (std::exception const& e)
            {
                TOXFS_LOG_ERROR("Exception while handling recevied message: {}", e.what());
            }
        }
    }
}

} // namespace toxfs::tox

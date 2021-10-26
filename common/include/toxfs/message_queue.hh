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

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace toxfs
{

/**
 * A very basic thread safe queue using a mutex and condition variable
 */
template<class T, std::size_t MaxSize>
class message_queue
{
public:
    message_queue() = default;

    /**
     * @brief check if empty, this is NOT thread safe as the queue
     *        could change after return
     * @return true if the queue is empty
     */
    bool empty() const noexcept
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief push a new message onto the queue
     * @param[in] v - message
     */
    void push(T const& v)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(v);
        lock.unlock();
        cond_var_.notify_one();
    }

    /**
     * @brief push a new message onto the queue, moving it
     * @param[in] v - message
     */
    void push(T&& v)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(std::forward<T>(v));
        lock.unlock();
        cond_var_.notify_one();
    }

    /**
     * @brief get a new message off the queue
     * @return message
     */
    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]() { return !queue_.empty(); });
        T ret = std::move(queue_.front());
        queue_.pop();
        return ret;
    }

    /**
     * @brief get a new message off the queue with timeout
     * @return optionaly a T if pop succeed
     */
    template <class Rep, class Period>
    std::optional<T> pop_timeout(std::chrono::duration<Rep, Period> const& rel_time)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait_for(lock, rel_time, [this]() { return !queue_.empty(); });
        T ret = std::move(queue_.front());
        queue_.pop();
        return ret;
    }

    // If ever needed these can be implemented, but I doubt it will...
    message_queue(message_queue const&) = delete;
    message_queue(message_queue &&) = delete;

    message_queue& operator=(message_queue const&) = delete;
    message_queue& operator=(message_queue &&) = delete;

private:
    mutable std::mutex mutex_{};
    mutable std::condition_variable cond_var_{};
    std::queue<T> queue_{};
};


} // namespace toxfs

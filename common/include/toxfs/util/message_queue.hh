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
 * A very basic locked queue
 */
template<class T>
class locked_queue
{
public:
    locked_queue() = default;

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
     * @brief push a new item onto the queue
     * @param[in] m - item
     */
    void push(T&& m)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::forward<T>(m));
    }

    /**
     * @brief try get a new item off the queue
     * @return T if succeeded, none if the queue was empty
     */
    std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!queue_.empty())
        {
            std::optional<T> ret{std::move(queue_.front())};
            queue_.pop();
            return ret;
        }
        else
        {
            return std::nullopt;
        }
    }

    // If ever needed these can be implemented, but I doubt it will...
    locked_queue(locked_queue const&) = delete;
    locked_queue(locked_queue &&) = delete;

    locked_queue& operator=(locked_queue const&) = delete;
    locked_queue& operator=(locked_queue &&) = delete;

private:
    mutable std::mutex mutex_{};
    std::queue<T> queue_{};
};


/**
 * A very basic thread safe queue for passing messages using condition variables
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
     * @brief push a new threadsafe onto the queue
     * @param[in] m - threadsafe
     */
    void push(T&& m)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_pop_.wait(lock, [this]() { return queue_.size() < MaxSize; });
        queue_.push(std::forward<T>(m));
        lock.unlock();
        cond_push_.notify_one();
    }

    /**
     * @brief push a new threadsafe onto the queue
     * @param[in] m - threadsafe
     * @param[in] timeout - the timeout duration
     * @return true
     */
    template <class Rep, class Period>
    bool push_timeout(T&& m, std::chrono::duration<Rep, Period> const& timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cond_pop_.wait_for(lock, timeout, [this]() { return queue_.size() < MaxSize; }))
        {
            return false;
        }
        queue_.push(std::forward<T>(m));
        lock.unlock();
        cond_push_.notify_one();
        return true;
    }

    /**
     * @brief push a new threadsafe onto the queue
     * @param[in] m - threadsafe
     * @param[in] timeout - the timeout time
     * @return true
     */
    template <class Clock, class Duration>
    bool push_until_time(T&& m, std::chrono::time_point<Clock, Duration> const& timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cond_pop_.wait_until(lock, timeout, [this]() { return queue_.size() < MaxSize; }))
        {
            return false;
        }
        queue_.push(std::forward<T>(m));
        lock.unlock();
        cond_push_.notify_one();
        return true;
    }

    /**
     * @brief get a new threadsafe off the queue
     * @return threadsafe
     */
    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_push_.wait(lock, [this]() { return !queue_.empty(); });
        T ret{std::move(queue_.front())};
        queue_.pop();
        lock.unlock();
        cond_pop_.notify_one();
        return ret;
    }

    /**
     * @brief try to get a new threadsafe off the queue
     * @return T if succeeded, none if queue is empty
     */
    std::optional<T> try_pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!queue_.empty())
        {
            std::optional<T> ret{std::move(queue_.front())};
            queue_.pop();
            lock.unlock();
            cond_pop_.notify_one();
            return ret;
        }
        else
        {
            return std::nullopt;
        }
    }

    /**
     * @brief get a new threadsafe off the queue with timeout
     * @param[in] timeout - the timeout duration
     * @return T if succeeded, none if timed out
     */
    template <class Rep, class Period>
    std::optional<T> pop_timeout(std::chrono::duration<Rep, Period> const& timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cond_push_.wait_for(lock, timeout, [this]() { return !queue_.empty(); }))
        {
            return std::nullopt;
        }
        std::optional<T> ret{std::move(queue_.front())};
        queue_.pop();
        lock.unlock();
        cond_pop_.notify_one();
        return ret;
    }

    /**
     * @brief get a new threadsafe off the queue until a certain time
     * @param[in] timeout - the timeout time
     * @return T if succeeded, none if timed out
     */
    template <class Clock, class Duration>
    std::optional<T> pop_until_time(std::chrono::time_point<Clock, Duration> const& timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cond_push_.wait_until(lock, timeout, [this]() { return !queue_.empty(); }))
        {
            return std::nullopt;
        }
        std::optional<T> ret{std::move(queue_.front())};
        queue_.pop();
        lock.unlock();
        cond_pop_.notify_one();
        return ret;
    }

    // If ever needed these can be implemented, but I doubt it will...
    message_queue(message_queue const&) = delete;
    message_queue(message_queue &&) = delete;

    message_queue& operator=(message_queue const&) = delete;
    message_queue& operator=(message_queue &&) = delete;

private:
    mutable std::mutex mutex_{};
    mutable std::condition_variable cond_push_{};
    mutable std::condition_variable cond_pop_{};
    std::queue<T> queue_{};
};

} // namespace toxfs

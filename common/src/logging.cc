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

#include "toxfs/logging.hh"

#include <fmt/color.h>
#include <fmt/core.h>

#include <atomic>
#include <iostream>
#include <string_view>
#include <cstdio>

namespace toxfs
{

namespace detail
{

const char* level_to_str(log_level_t level) noexcept
{
    switch (level)
    {
        case log_level_t::debug: return "DEBUG";
        case log_level_t::info: return "INFO";
        case log_level_t::warning: return "WARNING";
        case log_level_t::error: return "ERROR";
    }
    return "UNKNOWN";
}

constexpr fmt::text_style k_prefix_color = fmt::emphasis::bold | fmt::fg(fmt::terminal_color::white);

fmt::text_style level_to_color(log_level_t level) noexcept
{
    switch (level)
    {
        case log_level_t::debug:
            return fmt::emphasis::bold |
                fmt::fg(fmt::terminal_color::white);
        case log_level_t::info:
            return fmt::fg(fmt::terminal_color::white) |
                fmt::bg(fmt::terminal_color::blue);
        case log_level_t::warning:
            return fmt::fg(fmt::terminal_color::black) |
                fmt::bg(fmt::terminal_color::yellow);
        case log_level_t::error:
            return fmt::fg(fmt::terminal_color::white) |
                fmt::bg(fmt::terminal_color::red);
    }
    return {};
};

} // namespace detail

#ifdef TOXFS_LOG_COLOR
    constexpr bool k_useColor = true;
#else
    constexpr bool k_useColor = false;
#endif // ifdef TOXFS_LOG_COLOR

std::atomic<log_level_t> g_log_level(log_level_t::debug);

/**
 * @brief get the log level
 */
log_level_t get_log_level() noexcept
{
    return g_log_level.load(std::memory_order_relaxed);
}

/**
 * @brief set the log level
 */
void set_log_level(log_level_t level) noexcept
{
    g_log_level.store(level, std::memory_order_relaxed);
}

/**
 * @brief output the log message
 * @param[in] level - the level of the log message
 * @param[in] file - the file of the log message
 * @param[in] line - the line in file
 * @param[in] fmt_s
 * @param[in] args
 */
void log_output(log_level_t level, const char *file, int line, fmt::string_view fmt_s, fmt::format_args args) noexcept
{
    fmt::basic_memory_buffer<char, 512> buffer;

    try
    {
        auto* level_str = detail::level_to_str(level);
        if constexpr (k_useColor)
        {
            fmt::format_to(fmt::appender(buffer), detail::k_prefix_color, "[{}:{}] {}: ", file, line, level_str);
            fmt::vformat_to(fmt::appender(buffer), detail::level_to_color(level), fmt_s, args);
        }
        else
        {
            fmt::format_to(fmt::appender(buffer), "[{}:{}] {}: ", file, line, level_str);
            fmt::vformat_to(fmt::appender(buffer), fmt_s, args);
        }

        std::cout << std::string_view{buffer.data(), buffer.size()} << std::endl;
    }
    catch (...)
    {
        std::printf("Failed to format and output log from: %s:%d\n", file, line);
    }
}

} // namespace toxfs

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

#include "toxfs/util/filename.hh"

#include <fmt/format.h>

namespace toxfs
{

enum class log_level_t
{
    debug = 0,
    info,
    warning,
    error
};

/**
 * @brief get the log level
 */
log_level_t get_log_level() noexcept;

/**
 * @brief set the log level
 */
void set_log_level(log_level_t level) noexcept;

/**
 * @brief output the log message
 * @param[in] level - the level of the log message
 * @param[in] file - the file of the log message
 * @param[in] line - the line in file
 * @param[in] fmt_s
 * @param[in] args
 */
void log_output(log_level_t level, const char *file, int line, fmt::string_view fmt_s, fmt::format_args args) noexcept;

/**
 * @brief template helper for logging
 */
template <typename S, typename... Args>
inline void log(log_level_t level, const char* file, int line, const S& format, Args&&... args) noexcept
{
    if (level >= get_log_level())
    {
        log_output(level, file, line, format, fmt::make_args_checked<Args...>(format, args...));
    }
}

} // namespace toxfs

#define TOXFS_LOG_LEVEL(log_level_, format_str_, ...) \
    ::toxfs::log(log_level_, TOXFS_FILENAME, __LINE__, \
            FMT_STRING(format_str_), ##__VA_ARGS__)

#define TOXFS_LOG_DEBUG(format_str_, ...) \
    TOXFS_LOG_LEVEL(::toxfs::log_level_t::debug, format_str_, ##__VA_ARGS__)

#define TOXFS_LOG_INFO(format_str_, ...) \
    TOXFS_LOG_LEVEL(::toxfs::log_level_t::info, format_str_, ##__VA_ARGS__)

#define TOXFS_LOG_WARNING(format_str_, ...) \
    TOXFS_LOG_LEVEL(::toxfs::log_level_t::warning, format_str_, ##__VA_ARGS__)

#define TOXFS_LOG_ERROR(format_str_, ...) \
    TOXFS_LOG_LEVEL(::toxfs::log_level_t::error, format_str_, ##__VA_ARGS__)

# Copyright (C) 2021 by The Toxfs Project Contributers
# 
# This file is part of Toxfs.
# 
# Toxfs is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Toxfs is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Toxfs.  If not, see <https://www.gnu.org/licenses/>.

# Project wide compiler flags
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS ON)


set(TOXFS_DEBUG_FLAGS
    $<$<CONFIG:Debug>:-Og>
    $<$<CONFIG:Debug>:-g3>
)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    # $<$<CONFIG:Debug>:-Weffc++>
    $<$<CONFIG:Debug>:-Werror>
    ${TOXFS_DEBUG_FLAGS}
)

add_link_options(
    ${TOXFS_DEBUG_FLAGS}
)

add_compile_definitions(
    $<$<CONFIG:Debug>:TOXFS_DEBUG>
    $<$<CONFIG:Debug>:TOXFS_LOG_COLOR>
    # Uncomment to allow all toxcore logs to be logged
    # $<$<CONFIG:Debug>:TOXFS_ALL_TOXCORE_LOGS>
)

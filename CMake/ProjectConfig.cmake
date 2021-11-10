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

# Project config options

set(BUILD_TOXFSD ON CACHE BOOL "Build toxfsd")
set(BUILD_TOXFUSE ON CACHE BOOL "Build toxfuse")

# Put built all executables in build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "CMake Build Type" FORCE)
endif()

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

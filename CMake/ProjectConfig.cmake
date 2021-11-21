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

# DOWNLOAD_DEPS
# Allows choosing whether to automatically download a dependency
# if the system version is not available or too old.
#
# This is only for dependencies that support it. Currently this is
# just header-only libraries
#
# NEVER - Always use the system version, build fails if not found
# AUTO - The default, tries to find the system version, on failure it downloads.
# ALWAYS - Always download, without checking for the system one.
set(_DOWNLOAD_DEPS_OPTS NEVER AUTO ALWAYS)
set(DOWNLOAD_DEPS AUTO CACHE STRING
    "If dependencies should be downloaded from the web")
set_property(CACHE DOWNLOAD_DEPS PROPERTY STRINGS ${_DOWNLOAD_DEPS_OPTS})
if(NOT DOWNLOAD_DEPS IN_LIST _DOWNLOAD_DEPS_OPTS)
    message(FATAL_ERROR "Invalid value of DOWNLOAD_DEPS: ${DOWNLOAD_DEPS}")
endif()

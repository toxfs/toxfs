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
cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/CMake)

project(toxfs
    VERSION 0.1.0
    DESCRIPTION "Tox based remote filesystem"
    HOMEPAGE_URL TODO
    LANGUAGES CXX
)

include(ProjectConfig)
include(Dependencies)
include(Flags)

add_subdirectory(common EXCLUDE_FROM_ALL)

# TODO: check dependencies

if(BUILD_TOXFSD)
    add_subdirectory(toxfsd)
endif()

if(BUILD_TOXFUSE)
    add_subdirectory(toxfuse)
endif()

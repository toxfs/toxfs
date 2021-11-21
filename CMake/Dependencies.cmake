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

# Find dependencies
include(FetchContent)

if(DOWNLOAD_DEPS STREQUAL NEVER)
    set(_find_args REQUIRED)
    set(_do_find TRUE)
    set(_do_download FALSE)
elseif(DOWNLOAD_DEPS STREQUAL AUTO)
    set(_find_args)
    set(_do_find TRUE)
    set(_do_download TRUE)
elseif(DOWNLOAD_DEPS STREQUAL ALWAYS)
    set(_find_args INVALID_ARG)
    set(_do_find FALSE)
    set(_do_download TRUE)
endif()

# ================================
# pkg-config, needed for finding other modules
# ================================
find_package(PkgConfig REQUIRED)

# ================================
# Git (optional, for generating version)
# ================================
find_package(Git)

# ================================
# toxcore
# ================================
pkg_check_modules(toxcore toxcore>=0.2.10 IMPORTED_TARGET REQUIRED)
add_library(toxcore::toxcore INTERFACE IMPORTED)
target_link_libraries(toxcore::toxcore INTERFACE PkgConfig::toxcore)
set(TOXFS_toxcore_DEP_TYPE "System (${toxcore_VERSION})")

# ================================
# fmtlib
# ================================
add_library(toxfsdep::fmt INTERFACE IMPORTED)
if(_do_find)
    find_package(fmt 8.0.1 ${_find_args})
    if(fmt_FOUND)
        target_link_libraries(toxfsdep::fmt INTERFACE fmt::fmt)
        set(TOXFS_fmtlib_DEP_TYPE "System (${fmt_VERSION})")
    endif()
endif()
if(NOT fmt_FOUND AND _do_download)
    set(TOXFS_FMT_DOWNLOAD_VERSION 8.0.1)
    FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG ${TOXFS_FMT_DOWNLOAD_VERSION}
    )
    FetchContent_MakeAvailable(fmt)
    target_link_libraries(toxfsdep::fmt INTERFACE fmt)
    set(TOXFS_fmtlib_DEP_TYPE "Downloaded (${TOXFS_FMT_DOWNLOAD_VERSION})")
endif()

# Print out all dependencies
message(STATUS "Toxfs Dependencies Summary:")
foreach(dep IN ITEMS toxcore fmtlib GSL)
    message(STATUS "  ${dep}:\t${TOXFS_${dep}_DEP_TYPE}")
endforeach(dep)
message(STATUS "End of Toxfs Dependencies Summary")

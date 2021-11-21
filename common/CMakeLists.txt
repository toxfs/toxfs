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

# toxfs_common: common code between toxfsd and toxfuse

add_library(toxfs_common)

target_sources(toxfs_common PRIVATE
    include/toxfs/version.hh
    include/toxfs/logging.hh
    include/toxfs/exception.hh
    include/toxfs/tox/tox.hh
    include/toxfs/tox/tox_if.hh
    include/toxfs/tox/tox_types.hh
    include/toxfs/tox/tox_error.hh
    include/toxfs/util/string_helpers.hh
    include/toxfs/util/compile_utils.hh
    include/toxfs/util/message_queue.hh
    include/toxfs/util/filename.hh
    include_private/toxfs_priv/tox/tox_msg_types.hh
    include_private/toxfs_priv/cmake_version.hh.in
    src/version.cc
    src/logging.cc
    src/util/string_helpers.cc
    src/tox/tox.cc
    src/tox/tox_error.cc
)

target_include_directories(toxfs_common
    PUBLIC
    include

    PRIVATE
    include_private
    ${CMAKE_CURRENT_BINARY_DIR}/include_private
)

target_link_libraries(toxfs_common
    PUBLIC
    toxfsdep::GSL

    PRIVATE
    toxfsdep::fmt
    toxcore::toxcore
)

add_custom_target(toxfs_gen_version
    COMMAND ${CMAKE_COMMAND}
        -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/include_private/toxfs_priv/cmake_version.hh.in
        -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/include_private/toxfs_priv/cmake_version.hh.tmp
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DGIT_FOUND=${GIT_FOUND}
        -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
        -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        -DVERSION_MINOR=${PROJECT_VERSION_MINOR}
        -DVERSION_PATCH=${PROJECT_VERSION_PATCH}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/GenerateVersion.cmake
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${CMAKE_CURRENT_BINARY_DIR}/include_private/toxfs_priv/cmake_version.hh.tmp
        ${CMAKE_CURRENT_BINARY_DIR}/include_private/toxfs_priv/cmake_version.hh
    BYPRODUCTS 
        ${CMAKE_CURRENT_BINARY_DIR}/include_private/toxfs_priv/cmake_version.hh
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/include_private/toxfs_priv/cmake_version.hh.in
        ${CMAKE_CURRENT_SOURCE_DIR}/GenerateVersion.cmake
    COMMENT "Generating Version..."
)
add_dependencies(toxfs_common toxfs_gen_version)

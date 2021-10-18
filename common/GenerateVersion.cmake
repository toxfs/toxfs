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

if(NOT INPUT_FILE)
    message(FATAL_ERROR "INPUT_FILE not given")
endif()

if(NOT SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR not given")
endif()

if(NOT OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE not given")
endif()

if(GIT_FOUND AND GIT_EXECUTABLE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        RESULT_VARIABLE res
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${SOURCE_DIR}
    )

    if(NOT res EQUAL 0)
        message(FATAL_ERROR "Failed to run git rev-parse HEAD")
    endif()

    set(TOXFS_GIT_HASH TRUE)
endif()

configure_file(${INPUT_FILE} ${OUTPUT_FILE})

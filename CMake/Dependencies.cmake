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

# pkg-config, needed for finding other modules
find_package(PkgConfig REQUIRED)

# Git (optional, for generating version)
find_package(Git)

# toxcore
pkg_check_modules(toxcore toxcore>=0.2.10 IMPORTED_TARGET REQUIRED)
add_library(toxcore::toxcore INTERFACE IMPORTED)
target_link_libraries(toxcore::toxcore INTERFACE PkgConfig::toxcore)

# fmtlib
find_package(fmt 8 REQUIRED)

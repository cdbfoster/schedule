# This file is part of schedule.
#
# schedule is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# schedule is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with schedule. If not, see <http://www.gnu.org/licenses/>.
#
# Copyright 2015 Chris Foster

cmake_minimum_required(VERSION 2.8)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed!")
endif()

# OpenMemo Setup ==========================================

project(schedule)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Boost Setup =============================================

# Use -DBOOST_ROOT="path/to/boost" to use a custom installation of boost.

set(BOOST_COMPONENTS date_time)

if(BOOST_ROOT)
	set(Boost_NO_SYSTEM_PATHS TRUE)
endif()

find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

# Source ==================================================

set(include
	Activity.hpp
	Offset.hpp
	Schedule.hpp
	ScheduleFileIO.hpp
)

set(source
	Activity.cpp
	main.cpp
	Offset.cpp
	Schedule.cpp
	ScheduleFileIO.cpp
)

#include_directories(${Boost_INCLUDE_DIRS})

# Executable ==============================================

add_executable(schedule ${include} ${source})

#target_link_libraries(schedule ${Boost_LIBRARIES})

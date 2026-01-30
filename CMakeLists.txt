# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.12)

# project name
set(MINI_CHROMIUM_PROJECT_NAME mini_chromium)
project(${MINI_CHROMIUM_PROJECT_NAME})

#
set(CMAKE_CXX_STANDARD 14)

# macros
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(MINI_CHROMIUM_OS_WIN ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(MINI_CHROMIUM_OS_LINUX ON)
	set(MINI_CHROMIUM_OS_POSIX ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
	set(MINI_CHROMIUM_OS_NETBSD ON)
	set(MINI_CHROMIUM_OS_POSIX ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	set(MINI_CHROMIUM_OS_OPENBSD ON)
	set(MINI_CHROMIUM_OS_POSIX ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set(MINI_CHROMIUM_OS_FREEBSD ON)
	set(MINI_CHROMIUM_OS_POSIX ON)
endif()

# compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(MINI_CHROMIUM_COMPILER_MSVC ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(MINI_CHROMIUM_COMPILER_GCC ON)
endif()

# src dir
set(MINI_CHROMIUM_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MINI_CHROMIUM_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# e.g: cmake .. -G "Visual Studio 15 2017" -DMODE=DLL -DSTATIC=1 -DBUILD_TESTS=1

# project compiler option
if (MODE)
if (((${MODE} STREQUAL "so") OR (${MODE} STREQUAL "SO") OR
		(${MODE} STREQUAL "dll") OR (${MODE} STREQUAL "DLL")))
	set(MINI_CHROMIUM_COMPONENT_BUILD ON)
endif()
endif()

if (${STATIC})
	set(MINI_CHROMIUM_STATIC_CRT ON)
endif()

# sub project
add_subdirectory(${MINI_CHROMIUM_SRC_DIR}/crbase)
add_subdirectory(${MINI_CHROMIUM_SRC_DIR}/crbase_runtime)
add_subdirectory(${MINI_CHROMIUM_SRC_DIR}/crurl)

if (${BUILD_TESTS})
	add_subdirectory(${MINI_CHROMIUM_TESTS_DIR}/crbase_tests)
endif()

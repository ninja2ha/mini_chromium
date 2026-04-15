# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# compiler options

project(${MINI_CHROMIUM_PROJECT_NAME})
set(CREVENT_TESTS_SRC_DIR ${MINI_CHROMIUM_TESTS_DIR}/crevent_tests)

# gtests path
set(MINI_CHROMIUM_GTEST_SRC_DIR
    ${MINI_CHROMIUM_TESTS_DIR}/third_party/googletest-release-1.12.1
)

#
set(CREVENT_TESTS_SRC_FILES 
	${CREVENT_TESTS_SRC_DIR}/main.cc
)

add_executable(crevent_tests ${CREVENT_TESTS_SRC_FILES})
add_dependencies(crevent_tests crbase crevent)

if (${MINI_CHROMIUM_COMPONENT_BUILD})
	target_compile_definitions(crevent_tests PRIVATE 
		MINI_CHROMIUM_COMPONENT_BUILD)
	target_link_libraries(crevent_tests 
		crbase crevent)
else()
	if (${MINI_CHROMIUM_COMPILER_MSVC})
		target_link_libraries(crevent_tests 
			winmm version shlwapi ws2_32 crbase crevent)
	elseif(${MINI_CHROMIUM_COMPILER_GCC})
		target_link_libraries(crevent_tests 
			pthread crbase crevent)
	endif()
endif()

if(${MINI_CHROMIUM_STATIC_CRT})
	if(${MINI_CHROMIUM_COMPILER_MSVC})
		target_compile_options(crevent_tests PRIVATE 
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
		)
	endif()
endif()

# includes
target_include_directories(crevent_tests PRIVATE 
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googletest
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googletest/include
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googlemock
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googlemock/include
    ${MINI_CHROMIUM_SRC_DIR}
	${MINI_CHROMIUM_TESTS_DIR}
)


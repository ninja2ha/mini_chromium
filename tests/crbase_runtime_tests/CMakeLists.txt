# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# compiler options

project(${MINI_CHROMIUM_PROJECT_NAME})
set(CRBASE_RT_TESTS_SRC_DIR ${MINI_CHROMIUM_TESTS_DIR}/crbase_runtime_tests)

# gtests path
set(MINI_CHROMIUM_GTEST_SRC_DIR
    ${MINI_CHROMIUM_TESTS_DIR}/third_party/googletest-release-1.12.1
)

#
set(CRBASE_RT_TESTS_SRC_FILES 
	${CRBASE_RT_TESTS_SRC_DIR}/main.cc
)

add_executable(crbase_runtime_tests ${CRBASE_RT_TESTS_SRC_FILES})
add_dependencies(crbase_runtime_tests crbase crbase_runtime)

if (${MINI_CHROMIUM_COMPONENT_BUILD})
	target_compile_definitions(crbase_runtime_tests PRIVATE 
		MINI_CHROMIUM_COMPONENT_BUILD)
	target_link_libraries(crbase_runtime_tests 
		crbase crbase_runtime)
else()
	if (${MINI_CHROMIUM_COMPILER_MSVC})
		target_link_libraries(crbase_runtime_tests 
			winmm version shlwapi ws2_32 crbase crbase_runtime)
	elseif(${MINI_CHROMIUM_COMPILER_GCC})
		target_link_libraries(crbase_runtime_tests 
			pthread crbase crbase_runtime)
	endif()
endif()

if(${MINI_CHROMIUM_STATIC_CRT})
	if(${MINI_CHROMIUM_COMPILER_MSVC})
		target_compile_options(crbase_runtime_tests PRIVATE 
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
		)
	endif()
endif()

# includes
target_include_directories(crbase_runtime_tests PRIVATE 
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googletest
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googletest/include
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googlemock
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googlemock/include
    ${MINI_CHROMIUM_SRC_DIR}
	${MINI_CHROMIUM_TESTS_DIR}
)


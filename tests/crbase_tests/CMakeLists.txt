# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# compiler options

project(${MINI_CHROMIUM_PROJECT_NAME})
set(CRBASE_TEST_SRC_DIR ${MINI_CHROMIUM_TESTS_DIR}/crbase_tests)

# gtests path
set(MINI_CHROMIUM_GTEST_SRC_DIR
    ${MINI_CHROMIUM_TESTS_DIR}/third_party/googletest-release-1.12.1
)

#
set(CRBASE_TEST_SRC_FILES 
	${CRBASE_TEST_SRC_DIR}/at_exit_unittest.cc
	${CRBASE_TEST_SRC_DIR}/environment_unittest.cc
	${CRBASE_TEST_SRC_DIR}/guid_unittest.cc
	${CRBASE_TEST_SRC_DIR}/observer_list_unittest.cc
	${CRBASE_TEST_SRC_DIR}/version_unittest.cc
)

# test
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/test/gtest_util.h
	${CRBASE_TEST_SRC_DIR}/test/gtest_platform_test.h
	${CRBASE_TEST_SRC_DIR}/test/gtest_main.cc
	${CRBASE_TEST_SRC_DIR}/test/bind.h
	${CRBASE_TEST_SRC_DIR}/test/bind.cc
)

# files
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/files/file_path_unittest.cc
	${CRBASE_TEST_SRC_DIR}/files/memory_mapped_file_unittest.cc
	${CRBASE_TEST_SRC_DIR}/files/scoped_temp_dir_unittest.cc
)

# functional
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/functional/bind_unittest.cc
)

# strings
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/strings/strcat_unittest.cc
	${CRBASE_TEST_SRC_DIR}/strings/string_number_conversions_unittest.cc
	${CRBASE_TEST_SRC_DIR}/strings/string_piece_unittest.cc
	${CRBASE_TEST_SRC_DIR}/strings/string_split_unittest.cc
	${CRBASE_TEST_SRC_DIR}/strings/stringprintf_unittest.cc
)

# synchronization
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/synchronization/lock_unittest.cc
	${CRBASE_TEST_SRC_DIR}/synchronization/waitable_event_unittest.cc
)

# time
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/time/time_unittest.cc
)

# time - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/time/time_win_unittest.cc
)
endif()

# win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_TEST_SRC_FILES ${CRBASE_TEST_SRC_FILES}
	${CRBASE_TEST_SRC_DIR}/win/scoped_bstr_unittest.cc
	${CRBASE_TEST_SRC_DIR}/win/scoped_comptr_unittest.cc
	${CRBASE_TEST_SRC_DIR}/win/scoped_variant_unittest.cc
)
endif()

add_executable(crbase_tests ${CRBASE_TEST_SRC_FILES})
add_dependencies(crbase_tests crbase)

if (${MINI_CHROMIUM_COMPONENT_BUILD})
	target_compile_definitions(crbase_tests PRIVATE MINI_CHROMIUM_COMPONENT_BUILD)
	target_link_libraries(crbase_tests crbase)
else()
	if (${MINI_CHROMIUM_COMPILER_MSVC})
		target_link_libraries(crbase_tests winmm version shlwapi ws2_32 crbase)
	elseif(${MINI_CHROMIUM_COMPILER_GCC})
		target_link_libraries(crbase_tests pthread crbase)
	endif()
endif()

if(${MINI_CHROMIUM_STATIC_CRT})
	if(${MINI_CHROMIUM_COMPILER_MSVC})
		target_compile_options(crbase_tests PRIVATE 
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
		)
	endif()
endif()

# includes
target_include_directories(crbase_tests PRIVATE 
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googletest
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googletest/include
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googlemock
    ${MINI_CHROMIUM_GTEST_SRC_DIR}/googlemock/include
    ${MINI_CHROMIUM_SRC_DIR}
	${MINI_CHROMIUM_TESTS_DIR}
)


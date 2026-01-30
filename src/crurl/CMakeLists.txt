# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# compiler options

project(${MINI_CHROMIUM_PROJECT_NAME})
set(CRURL_SRC_DIR ${MINI_CHROMIUM_SRC_DIR}/crurl)

set(CRURL_SRC_FILES
	${CRURL_SRC_DIR}/third_party/mozilla/url_parse.cc
	${CRURL_SRC_DIR}/third_party/mozilla/url_parse.h
	${CRURL_SRC_DIR}/gurl.cc
	${CRURL_SRC_DIR}/gurl.h
	${CRURL_SRC_DIR}/origin.cc
	${CRURL_SRC_DIR}/origin.h
	${CRURL_SRC_DIR}/scheme_host_port.cc
	${CRURL_SRC_DIR}/scheme_host_port.h
	${CRURL_SRC_DIR}/url_canon.cc
	${CRURL_SRC_DIR}/url_canon.h
	${CRURL_SRC_DIR}/url_canon_etc.cc
	${CRURL_SRC_DIR}/url_canon_filesystemurl.cc
	${CRURL_SRC_DIR}/url_canon_fileurl.cc
	${CRURL_SRC_DIR}/url_canon_host.cc
	${CRURL_SRC_DIR}/url_canon_internal.cc
	${CRURL_SRC_DIR}/url_canon_internal.h
	${CRURL_SRC_DIR}/url_canon_internal_file.h
	${CRURL_SRC_DIR}/url_canon_ip.cc
	${CRURL_SRC_DIR}/url_canon_ip.h
	${CRURL_SRC_DIR}/url_canon_mailtourl.cc
	${CRURL_SRC_DIR}/url_canon_path.cc
	${CRURL_SRC_DIR}/url_canon_pathurl.cc
	${CRURL_SRC_DIR}/url_canon_query.cc
	${CRURL_SRC_DIR}/url_canon_relative.cc
	${CRURL_SRC_DIR}/url_canon_stdstring.cc
	${CRURL_SRC_DIR}/url_canon_stdstring.h
	${CRURL_SRC_DIR}/url_canon_stdurl.cc
	${CRURL_SRC_DIR}/url_constants.cc
	${CRURL_SRC_DIR}/url_constants.h
	${CRURL_SRC_DIR}/url_export.h
	${CRURL_SRC_DIR}/url_file.h
	${CRURL_SRC_DIR}/url_parse_file.cc
	${CRURL_SRC_DIR}/url_parse_internal.h
	${CRURL_SRC_DIR}/url_util.cc
	${CRURL_SRC_DIR}/url_util.h
	${CRURL_SRC_DIR}/url_util_internal.cc
)

# compiler options#
if (${MINI_CHROMIUM_COMPONENT_BUILD})
	add_library(crurl SHARED ${CRURL_SRC_FILES})
	add_dependencies(crurl crbase)

	# macros
	target_compile_definitions(crurl PRIVATE 
		MINI_CHROMIUM_COMPONENT_BUILD 
		CRURL_IMPLEMENTATION)

	target_link_libraries(crurl crbase)
else()
	add_library(crurl STATIC ${CRBASE_SRC_FILES})
	add_dependencies(crurl crbase)
endif()

if(${MINI_CHROMIUM_STATIC_CRT})
	if(${MINI_CHROMIUM_COMPILER_MSVC})
		target_compile_options(crurl PRIVATE 
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
		)
	endif()
endif()

target_include_directories(crurl PRIVATE ${MINI_CHROMIUM_SRC_DIR})

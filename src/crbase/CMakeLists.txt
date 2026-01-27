# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# compiler options

project(${MINI_CHROMIUM_PROJECT_NAME})
set(CRBASE_SRC_DIR ${MINI_CHROMIUM_SRC_DIR}/crbase)

set(CRBASE_SRC_FILES 
	${CRBASE_SRC_DIR}/at_exit.cc
	${CRBASE_SRC_DIR}/at_exit.h
	${CRBASE_SRC_DIR}/auto_reset.h
	${CRBASE_SRC_DIR}/base_export.h
	${CRBASE_SRC_DIR}/bit_cast.h
	${CRBASE_SRC_DIR}/bits.h
	${CRBASE_SRC_DIR}/environment.cc
	${CRBASE_SRC_DIR}/environment.h
	${CRBASE_SRC_DIR}/guid.cc
	${CRBASE_SRC_DIR}/guid.h
	${CRBASE_SRC_DIR}/location.cc
	${CRBASE_SRC_DIR}/location.h
	${CRBASE_SRC_DIR}/observer_list.h
	${CRBASE_SRC_DIR}/observer_list_type.cc
	${CRBASE_SRC_DIR}/observer_list_type.h
	${CRBASE_SRC_DIR}/rand_util.cc
	${CRBASE_SRC_DIR}/rand_util.h
	${CRBASE_SRC_DIR}/stl_util.h
	${CRBASE_SRC_DIR}/sys_byteorder.h
	${CRBASE_SRC_DIR}/version.cc
	${CRBASE_SRC_DIR}/version.h
)

# atomic
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/atomic/atomic_ref_count.h
	${CRBASE_SRC_DIR}/atomic/atomic_sequence_num.h
	${CRBASE_SRC_DIR}/atomic/atomicops.h
	${CRBASE_SRC_DIR}/atomic/atomicops_internals_atomicword_compat.h
	${CRBASE_SRC_DIR}/atomic/atomicops_internals_portable.h
)

# containers
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/containers/circular_deque.h
	${CRBASE_SRC_DIR}/containers/optional.h
	${CRBASE_SRC_DIR}/containers/span.h
	${CRBASE_SRC_DIR}/containers/stack.h
	${CRBASE_SRC_DIR}/containers/util.h
	${CRBASE_SRC_DIR}/containers/internal/checked_iterators.h
	${CRBASE_SRC_DIR}/containers/internal/contiguous_iterator.h
	${CRBASE_SRC_DIR}/containers/internal/vector_buffer.h
)

# debug
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/debug/alias.cc
	${CRBASE_SRC_DIR}/debug/alias.h
	${CRBASE_SRC_DIR}/debug/immediate_crash.cc
	${CRBASE_SRC_DIR}/debug/immediate_crash.h
)

# files
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/files/file.cc
	${CRBASE_SRC_DIR}/files/file.h
	${CRBASE_SRC_DIR}/files/file_enumerator.cc
	${CRBASE_SRC_DIR}/files/file_enumerator.h
	${CRBASE_SRC_DIR}/files/file_path.cc
	${CRBASE_SRC_DIR}/files/file_path.h
	${CRBASE_SRC_DIR}/files/file_path_constants.cc
	${CRBASE_SRC_DIR}/files/file_util.cc
	${CRBASE_SRC_DIR}/files/file_util.h
	${CRBASE_SRC_DIR}/files/file_version_info.h	
	${CRBASE_SRC_DIR}/files/memory_mapped_file.cc
	${CRBASE_SRC_DIR}/files/memory_mapped_file.h
	${CRBASE_SRC_DIR}/files/platform_file.h
	${CRBASE_SRC_DIR}/files/scoped_file.h
	${CRBASE_SRC_DIR}/files/scoped_temp_dir.cc
	${CRBASE_SRC_DIR}/files/scoped_temp_dir.h
)

# files - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/files/win/file_enumerator_win.cc
	${CRBASE_SRC_DIR}/files/win/file_win.cc
	${CRBASE_SRC_DIR}/files/win/file_util_win.cc
	${CRBASE_SRC_DIR}/files/win/file_version_info_win.h
	${CRBASE_SRC_DIR}/files/win/file_version_info_win.cc
	${CRBASE_SRC_DIR}/files/win/memory_mapped_file_win.cc
)
endif()

# files - posix
if (${MINI_CHROMIUM_OS_POSIX})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/files/posix/file_enumerator_posix.cc
	${CRBASE_SRC_DIR}/files/posix/file_util_posix.cc
	${CRBASE_SRC_DIR}/files/posix/file_posix.cc
	${CRBASE_SRC_DIR}/files/posix/memory_mapped_file_posix.cc
	${CRBASE_SRC_DIR}/files/scoped_file.cc
)
endif()

# functional
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/functional/bind.h
	${CRBASE_SRC_DIR}/functional/bind_internal.h
	${CRBASE_SRC_DIR}/functional/callback.h
	${CRBASE_SRC_DIR}/functional/callback_forward.h
	${CRBASE_SRC_DIR}/functional/callback_internal.cc
	${CRBASE_SRC_DIR}/functional/callback_internal.h
	${CRBASE_SRC_DIR}/functional/cancelable_callback.h
)

# helper
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/helper/scoped_generic.h
	${CRBASE_SRC_DIR}/helper/scoped_clear_last_error.h
)

# helper - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/helper/win/scoped_clear_last_error_win.cc
)
endif()

# internal
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/internal/template_util.h
	${CRBASE_SRC_DIR}/internal/raw_ref_ptr_mismatch_checker.h
	${CRBASE_SRC_DIR}/internal/lazy_instance_helpers.cc
	${CRBASE_SRC_DIR}/internal/lazy_instance_helpers.h
	${CRBASE_SRC_DIR}/internal/observer_list_internal.cc
	${CRBASE_SRC_DIR}/internal/observer_list_internal.h
	${CRBASE_SRC_DIR}/internal/gtest_prod_util.h
)

# logging
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/logging/logging.cc
	${CRBASE_SRC_DIR}/logging/logging.h
	${CRBASE_SRC_DIR}/logging/logging_types.h
)

# logging - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/logging/win/logging_internal_win.cc
	${CRBASE_SRC_DIR}/logging/win/logging_internal_win.h
)
endif()

# logging - posix
if (${MINI_CHROMIUM_OS_POSIX})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/logging/posix/logging_internal_posix.cc
	${CRBASE_SRC_DIR}/logging/posix/logging_internal_posix.h
)
endif()

# memory
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/memory/no_destructor.h
	${CRBASE_SRC_DIR}/memory/ptr_util.h
	${CRBASE_SRC_DIR}/memory/ref_counted.cc
	${CRBASE_SRC_DIR}/memory/ref_counted.h
	${CRBASE_SRC_DIR}/memory/ref_ptr.h
	${CRBASE_SRC_DIR}/memory/singleton.h
	${CRBASE_SRC_DIR}/memory/weak_ptr.cc
	${CRBASE_SRC_DIR}/memory/weak_ptr.h
)

# numerics
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/numerics/checked_math.h
	${CRBASE_SRC_DIR}/numerics/checked_math_impl.h
	${CRBASE_SRC_DIR}/numerics/clamped_math.h
	${CRBASE_SRC_DIR}/numerics/clamped_math_impl.h
	${CRBASE_SRC_DIR}/numerics/safe_conversions.h
	${CRBASE_SRC_DIR}/numerics/safe_conversions_arm_impl.h
	${CRBASE_SRC_DIR}/numerics/safe_conversions_impl.h
	${CRBASE_SRC_DIR}/numerics/ranges.h
	${CRBASE_SRC_DIR}/numerics/safe_math.h
	${CRBASE_SRC_DIR}/numerics/safe_math_arm_impl.h
	${CRBASE_SRC_DIR}/numerics/safe_math_clang_gcc_impl.h
	${CRBASE_SRC_DIR}/numerics/safe_math_shared_impl.h
)

# posix
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/posix/eintr_wrapper.h
	${CRBASE_SRC_DIR}/posix/file_descriptor_posix.h
	${CRBASE_SRC_DIR}/posix/rand_util_posix.cc
)

# strings
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/strings/char_traits.h
	${CRBASE_SRC_DIR}/strings/format_macros.h
	${CRBASE_SRC_DIR}/strings/strcat.cc
	${CRBASE_SRC_DIR}/strings/strcat.h
	${CRBASE_SRC_DIR}/strings/strcat_internal.h
	${CRBASE_SRC_DIR}/strings/string_number_conversions.cc
	${CRBASE_SRC_DIR}/strings/string_number_conversions.h
	${CRBASE_SRC_DIR}/strings/string_number_conversions_internal.h
	${CRBASE_SRC_DIR}/strings/string_piece.cc
	${CRBASE_SRC_DIR}/strings/string_piece.h
	${CRBASE_SRC_DIR}/strings/string_split.cc
	${CRBASE_SRC_DIR}/strings/string_split.h
	${CRBASE_SRC_DIR}/strings/string_split_internal.h
	${CRBASE_SRC_DIR}/strings/string_util.cc
	${CRBASE_SRC_DIR}/strings/string_util.h
	${CRBASE_SRC_DIR}/strings/string_util_constants.cc
	${CRBASE_SRC_DIR}/strings/string_util_internal.h
	${CRBASE_SRC_DIR}/strings/stringprintf.cc
	${CRBASE_SRC_DIR}/strings/stringprintf.h
	${CRBASE_SRC_DIR}/strings/sys_string_conversions.h
	${CRBASE_SRC_DIR}/strings/utf_string_conversion_utils.cc
	${CRBASE_SRC_DIR}/strings/utf_string_conversion_utils.h
	${CRBASE_SRC_DIR}/strings/utf_string_conversions.cc
	${CRBASE_SRC_DIR}/strings/utf_string_conversions.h
)

if (${MINI_CHROMIUM_OS_WIN})
# strings - win
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/strings/win/string_util_win.h
	${CRBASE_SRC_DIR}/strings/win/sys_string_conversions_win.cc
)
endif()

if (${MINI_CHROMIUM_OS_POSIX})
# strings - posix
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/strings/posix/string_util_posix.h
	${CRBASE_SRC_DIR}/strings/posix/sys_string_conversions_posix.cc
)
endif()

# synchronization
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/synchronization/atomic_flag.cc
	${CRBASE_SRC_DIR}/synchronization/atomic_flag.h
	${CRBASE_SRC_DIR}/synchronization/condition_variable.h
	${CRBASE_SRC_DIR}/synchronization/lock.cc
	${CRBASE_SRC_DIR}/synchronization/lock.h
	${CRBASE_SRC_DIR}/synchronization/lock_impl.h
	${CRBASE_SRC_DIR}/synchronization/waitable_event.h
)

# synchronization - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/synchronization/win/condition_variable_win.cc
	${CRBASE_SRC_DIR}/synchronization/win/lock_impl_win.cc
	${CRBASE_SRC_DIR}/synchronization/win/waitable_event_win.cc
)
endif()

# synchronization - posix
if (${MINI_CHROMIUM_OS_POSIX})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/synchronization/posix/condition_variable_posix.cc
	${CRBASE_SRC_DIR}/synchronization/posix/lock_impl_posix.cc
	${CRBASE_SRC_DIR}/synchronization/posix/waitable_event_posix.cc
)
endif()

# system
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/system/cpu_info.cc
	${CRBASE_SRC_DIR}/system/cpu_info.h
)

# threading
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/threading/platform_thread.cc
	${CRBASE_SRC_DIR}/threading/platform_thread.h
	${CRBASE_SRC_DIR}/threading/sequence_checker.h
	${CRBASE_SRC_DIR}/threading/sequence_checker_impl.cc
	${CRBASE_SRC_DIR}/threading/sequence_checker_impl.h
	${CRBASE_SRC_DIR}/threading/sequence_token.cc
	${CRBASE_SRC_DIR}/threading/sequence_token.h
	${CRBASE_SRC_DIR}/threading/thread_local.h
	${CRBASE_SRC_DIR}/threading/thread_local_internal.h
	${CRBASE_SRC_DIR}/threading/thread_local_storage.cc
	${CRBASE_SRC_DIR}/threading/thread_local_storage.h
)

# threading - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/threading/win/thread_local_storage_win.cc
)
endif()

# threading - posix
if (${MINI_CHROMIUM_OS_POSIX})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/threading/posix/thread_local_storage_posix.cc
)
endif()

# time
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/time/clock.cc
	${CRBASE_SRC_DIR}/time/clock.h
	${CRBASE_SRC_DIR}/time/default_clock.cc
	${CRBASE_SRC_DIR}/time/default_clock.h
	${CRBASE_SRC_DIR}/time/default_tick_clock.cc
	${CRBASE_SRC_DIR}/time/default_tick_clock.h
	${CRBASE_SRC_DIR}/time/tick_clock.cc
	${CRBASE_SRC_DIR}/time/tick_clock.h
	${CRBASE_SRC_DIR}/time/time.cc
	${CRBASE_SRC_DIR}/time/time.h
	${CRBASE_SRC_DIR}/time/time_override.cc
	${CRBASE_SRC_DIR}/time/time_override.h
	${CRBASE_SRC_DIR}/time/time_to_iso8601.cc
	${CRBASE_SRC_DIR}/time/time_to_iso8601.h
)

# time - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/time/win/time_win.cc
)
endif()

# time - posix
if (${MINI_CHROMIUM_OS_POSIX})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/time/posix/time_conversion_posix.cc
	${CRBASE_SRC_DIR}/time/posix/time_exploded_posix.cc
	${CRBASE_SRC_DIR}/time/posix/time_now_posix.cc
)
endif()

# win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/win/rand_util_win.cc
	${CRBASE_SRC_DIR}/win/registry.cc
	${CRBASE_SRC_DIR}/win/registry.h
	${CRBASE_SRC_DIR}/win/resource_util.cc
	${CRBASE_SRC_DIR}/win/resource_util.h
	${CRBASE_SRC_DIR}/win/scoped_handle.cc
	${CRBASE_SRC_DIR}/win/scoped_handle.h
	${CRBASE_SRC_DIR}/win/shlwapi.h
	${CRBASE_SRC_DIR}/win/win_api_helper.cc
	${CRBASE_SRC_DIR}/win/win_api_helper.h
	${CRBASE_SRC_DIR}/win/win_util.cc
	${CRBASE_SRC_DIR}/win/win_util.h
	${CRBASE_SRC_DIR}/win/windows_types.h
	${CRBASE_SRC_DIR}/win/windows_version.cc
	${CRBASE_SRC_DIR}/win/windows_version.h
)

# win - com
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
	${CRBASE_SRC_DIR}/win/com/scoped_bstr.cc
	${CRBASE_SRC_DIR}/win/com/scoped_bstr.h
	${CRBASE_SRC_DIR}/win/com/scoped_co_mem.h
	${CRBASE_SRC_DIR}/win/com/scoped_com_initializer.h
	${CRBASE_SRC_DIR}/win/com/scoped_comptr.h
	${CRBASE_SRC_DIR}/win/com/scoped_propvariant.h
	${CRBASE_SRC_DIR}/win/com/scoped_variant.cc
	${CRBASE_SRC_DIR}/win/com/scoped_variant.h
)
endif()

# third_party - double-conversion
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/bignum.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/bignum.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/bignum-dtoa.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/bignum-dtoa.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/cached-powers.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/cached-powers.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/diy-fp.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/double-conversion.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/double-to-string.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/double-to-string.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/fast-dtoa.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/fast-dtoa.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/fixed-dtoa.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/fixed-dtoa.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/ieee.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/string-to-double.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/string-to-double.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/strtod.h
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/strtod.cc
    ${CRBASE_SRC_DIR}/third_party/double_conversion/double-conversion/utils.h
)

# third_party - icu
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
    ${CRBASE_SRC_DIR}/third_party/icu/icu_utf.h
)

# third_party - nspr
set(CRBASE_SRC_FILES ${CRBASE_SRC_FILES} 
    ${CRBASE_SRC_DIR}/third_party/nspr/prtime.h
    ${CRBASE_SRC_DIR}/third_party/nspr/prtime.cc
)

# compiler options#
if (${MINI_CHROMIUM_COMPONENT_BUILD})
	add_library(crbase SHARED ${CRBASE_SRC_FILES})
	target_compile_definitions(crbase PRIVATE 
		MINI_CHROMIUM_IMPLEMENTATION MINI_CHROMIUM_COMPONENT_BUILD)
	if (${MINI_CHROMIUM_COMPILER_MSVC})
		target_link_libraries(crbase winmm version shlwapi ws2_32)
	elseif(${MINI_CHROMIUM_COMPILER_GCC})
		target_link_libraries(crbase pthread)
	endif()
else()
	add_library(crbase STATIC ${CRBASE_SRC_FILES})
endif()

if(${MINI_CHROMIUM_STATIC_CRT})
	if(${MINI_CHROMIUM_COMPILER_MSVC})
		target_compile_options(crbase PRIVATE 
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
		)
	endif()
endif()

target_include_directories(crbase PRIVATE ${MINI_CHROMIUM_SRC_DIR})

# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# compiler options

project(${MINI_CHROMIUM_PROJECT_NAME})
set(CRBASE_RT_SRC_DIR ${MINI_CHROMIUM_SRC_DIR}/crbase_runtime)

set(CRBASE_RT_SRC_FILES 
	${CRBASE_RT_SRC_DIR}/io_buffer.cc
	${CRBASE_RT_SRC_DIR}/io_buffer.h
	${CRBASE_RT_SRC_DIR}/parameter_pack.h
	${CRBASE_RT_SRC_DIR}/pending_task.cc
	${CRBASE_RT_SRC_DIR}/pending_task.h
	${CRBASE_RT_SRC_DIR}/run_loop.cc
	${CRBASE_RT_SRC_DIR}/run_loop.h
	${CRBASE_RT_SRC_DIR}/sequenced_task_runner.cc
	${CRBASE_RT_SRC_DIR}/sequenced_task_runner.h
	${CRBASE_RT_SRC_DIR}/sequenced_task_runner_helpers.h
	${CRBASE_RT_SRC_DIR}/single_thread_task_runner.h
	${CRBASE_RT_SRC_DIR}/task_runner.cc
	${CRBASE_RT_SRC_DIR}/task_runner.h
	${CRBASE_RT_SRC_DIR}/traits_bag.h
)

# containers
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/containers/intrusive_heap.cc
	${CRBASE_RT_SRC_DIR}/containers/intrusive_heap.h
)

# internal
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/internal/post_task_and_reply_with_result_internal.h
)

# message_pump
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump.cc
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump.h
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_default.cc
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_default.h
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_for_io.h
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_for_ui.h
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_type.h
	${CRBASE_RT_SRC_DIR}/message_pump/timer_slack.h
	${CRBASE_RT_SRC_DIR}/message_pump/watchable_io_message_pump_posix.cc
	${CRBASE_RT_SRC_DIR}/message_pump/watchable_io_message_pump_posix.h
	${CRBASE_RT_SRC_DIR}/message_pump/work_id_provider.cc
	${CRBASE_RT_SRC_DIR}/message_pump/work_id_provider.h
)

# message_pump - win
if (${MINI_CHROMIUM_OS_WIN})
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_win.cc
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_win.h
)
endif()

# message_pump - linux
if (${MINI_CHROMIUM_OS_LINUX})
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_epoll.cc
	${CRBASE_RT_SRC_DIR}/message_pump/message_pump_epoll.h
)
endif()

# posix
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/posix/file_descriptor_watcher_posix.cc
	${CRBASE_RT_SRC_DIR}/posix/file_descriptor_watcher_posix.h
)

# task
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/task/current_thread.cc
	${CRBASE_RT_SRC_DIR}/task/current_thread.h
	${CRBASE_RT_SRC_DIR}/task/simple_task_executor.cc
	${CRBASE_RT_SRC_DIR}/task/simple_task_executor.h
	${CRBASE_RT_SRC_DIR}/task/single_thread_task_executor.cc
	${CRBASE_RT_SRC_DIR}/task/single_thread_task_executor.h
	${CRBASE_RT_SRC_DIR}/task/single_thread_task_runner_thread_mode.h
	${CRBASE_RT_SRC_DIR}/task/task_executor.cc
	${CRBASE_RT_SRC_DIR}/task/task_executor.h
	${CRBASE_RT_SRC_DIR}/task/task_observer.h
	${CRBASE_RT_SRC_DIR}/task/task_traits.h
	${CRBASE_RT_SRC_DIR}/task/task_traits_extension.h
)

# task - common
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/task/common/checked_lock.h
	${CRBASE_RT_SRC_DIR}/task/common/checked_lock_impl.cc
	${CRBASE_RT_SRC_DIR}/task/common/checked_lock_impl.h
	${CRBASE_RT_SRC_DIR}/task/common/intrusive_heap.h
	${CRBASE_RT_SRC_DIR}/task/common/operations_controller.cc
	${CRBASE_RT_SRC_DIR}/task/common/operations_controller.h
	${CRBASE_RT_SRC_DIR}/task/common/scoped_defer_task_posting.cc
	${CRBASE_RT_SRC_DIR}/task/common/scoped_defer_task_posting.h
)

# task - common
set(CRBASE_RT_SRC_FILES ${CRBASE_RT_SRC_FILES}
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/associated_thread_id.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/associated_thread_id.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/atomic_flag_set.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/atomic_flag_set.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/enqueue_order.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/enqueue_order_generator.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/enqueue_order_generator.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/lazily_deallocated_deque.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/lazy_now.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/lazy_now.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/real_time_domain.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/real_time_domain.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/sequence_manager.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/sequence_manager.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/sequence_manager_impl.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/sequence_manager_impl.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/sequenced_task_source.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_queue.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_queue.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_queue_impl.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_queue_impl.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_queue_selector.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_queue_selector.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_queue_selector_logic.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/task_time_observer.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/tasks.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/tasks.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/thread_controller.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/thread_controller.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/thread_controller_impl.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/thread_controller_impl.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/thread_controller_with_message_pump_impl.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/thread_controller_with_message_pump_impl.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/time_domain.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/time_domain.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/work_deduplicator.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/work_deduplicator.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/work_queue.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/work_queue.h
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/work_queue_sets.cc
	${CRBASE_RT_SRC_DIR}/task/sequence_manager/work_queue_sets.h
)

# compiler options#
if (${MINI_CHROMIUM_COMPONENT_BUILD})
	add_library(crbase_runtime SHARED ${CRBASE_RT_SRC_FILES})
	add_dependencies(crbase_runtime crbase)

	# macros
	target_compile_definitions(crbase_runtime PRIVATE 
		MINI_CHROMIUM_COMPONENT_BUILD 
		CRBASE_RT_IMPLEMENTATION)

	target_link_libraries(crbase_runtime crbase)
else()
	add_library(crbase_runtime STATIC ${CRBASE_RT_SRC_FILES})
	add_dependencies(crbase_runtime crbase)
endif()

if(${MINI_CHROMIUM_STATIC_CRT})
	if(${MINI_CHROMIUM_COMPILER_MSVC})
		target_compile_options(crbase_runtime PRIVATE 
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
		)
	endif()
endif()

target_include_directories(crbase_runtime PRIVATE ${MINI_CHROMIUM_SRC_DIR})

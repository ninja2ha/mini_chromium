# Copyright (c) 2013 Ninja2ha. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# compiler options

project(${MINI_CHROMIUM_PROJECT_NAME})
set(CREVENT_SRC_DIR ${MINI_CHROMIUM_SRC_DIR}/crevent)

set(CREVENT_SRC_FILES 
	${CREVENT_SRC_DIR}/io_buffer.cc
	${CREVENT_SRC_DIR}/io_buffer.h
	${CREVENT_SRC_DIR}/parameter_pack.h
	${CREVENT_SRC_DIR}/pending_task.cc
	${CREVENT_SRC_DIR}/pending_task.h
	${CREVENT_SRC_DIR}/run_loop.cc
	${CREVENT_SRC_DIR}/run_loop.h
	${CREVENT_SRC_DIR}/sequenced_task_runner.cc
	${CREVENT_SRC_DIR}/sequenced_task_runner.h
	${CREVENT_SRC_DIR}/sequenced_task_runner_helpers.h
	${CREVENT_SRC_DIR}/single_thread_task_runner.h
	${CREVENT_SRC_DIR}/task_runner.cc
	${CREVENT_SRC_DIR}/task_runner.h
	${CREVENT_SRC_DIR}/traits_bag.h
)

# containers
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/containers/intrusive_heap.cc
	${CREVENT_SRC_DIR}/containers/intrusive_heap.h
)

# internal
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/internal/post_task_and_reply_with_result_internal.h
)

# message_pump
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/message_pump/message_pump.cc
	${CREVENT_SRC_DIR}/message_pump/message_pump.h
	${CREVENT_SRC_DIR}/message_pump/message_pump_default.cc
	${CREVENT_SRC_DIR}/message_pump/message_pump_default.h
	${CREVENT_SRC_DIR}/message_pump/message_pump_for_io.h
	${CREVENT_SRC_DIR}/message_pump/message_pump_for_ui.h
	${CREVENT_SRC_DIR}/message_pump/message_pump_type.h
	${CREVENT_SRC_DIR}/message_pump/timer_slack.h
	${CREVENT_SRC_DIR}/message_pump/watchable_io_message_pump_posix.cc
	${CREVENT_SRC_DIR}/message_pump/watchable_io_message_pump_posix.h
	${CREVENT_SRC_DIR}/message_pump/work_id_provider.cc
	${CREVENT_SRC_DIR}/message_pump/work_id_provider.h
)

# message_pump - win
if (${MINI_CHROMIUM_OS_WIN})
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/message_pump/message_pump_win.cc
	${CREVENT_SRC_DIR}/message_pump/message_pump_win.h
)
endif()

# message_pump - linux
if (${MINI_CHROMIUM_OS_LINUX})
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/message_pump/message_pump_epoll.cc
	${CREVENT_SRC_DIR}/message_pump/message_pump_epoll.h
)
endif()

# posix
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/posix/file_descriptor_watcher_posix.cc
	${CREVENT_SRC_DIR}/posix/file_descriptor_watcher_posix.h
)

# task
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/task/current_thread.cc
	${CREVENT_SRC_DIR}/task/current_thread.h
	${CREVENT_SRC_DIR}/task/simple_task_executor.cc
	${CREVENT_SRC_DIR}/task/simple_task_executor.h
	${CREVENT_SRC_DIR}/task/single_thread_task_executor.cc
	${CREVENT_SRC_DIR}/task/single_thread_task_executor.h
	${CREVENT_SRC_DIR}/task/single_thread_task_runner_thread_mode.h
	${CREVENT_SRC_DIR}/task/task_executor.cc
	${CREVENT_SRC_DIR}/task/task_executor.h
	${CREVENT_SRC_DIR}/task/task_observer.h
	${CREVENT_SRC_DIR}/task/task_traits.h
	${CREVENT_SRC_DIR}/task/task_traits_extension.h
)

# task - common
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/task/common/checked_lock.h
	${CREVENT_SRC_DIR}/task/common/checked_lock_impl.cc
	${CREVENT_SRC_DIR}/task/common/checked_lock_impl.h
	${CREVENT_SRC_DIR}/task/common/intrusive_heap.h
	${CREVENT_SRC_DIR}/task/common/operations_controller.cc
	${CREVENT_SRC_DIR}/task/common/operations_controller.h
	${CREVENT_SRC_DIR}/task/common/scoped_defer_task_posting.cc
	${CREVENT_SRC_DIR}/task/common/scoped_defer_task_posting.h
)

# task - common
set(CREVENT_SRC_FILES ${CREVENT_SRC_FILES}
	${CREVENT_SRC_DIR}/task/sequence_manager/associated_thread_id.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/associated_thread_id.h
	${CREVENT_SRC_DIR}/task/sequence_manager/atomic_flag_set.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/atomic_flag_set.h
	${CREVENT_SRC_DIR}/task/sequence_manager/enqueue_order.h
	${CREVENT_SRC_DIR}/task/sequence_manager/enqueue_order_generator.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/enqueue_order_generator.h
	${CREVENT_SRC_DIR}/task/sequence_manager/lazily_deallocated_deque.h
	${CREVENT_SRC_DIR}/task/sequence_manager/lazy_now.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/lazy_now.h
	${CREVENT_SRC_DIR}/task/sequence_manager/real_time_domain.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/real_time_domain.h
	${CREVENT_SRC_DIR}/task/sequence_manager/sequence_manager.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/sequence_manager.h
	${CREVENT_SRC_DIR}/task/sequence_manager/sequence_manager_impl.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/sequence_manager_impl.h
	${CREVENT_SRC_DIR}/task/sequence_manager/sequenced_task_source.h
	${CREVENT_SRC_DIR}/task/sequence_manager/task_queue.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/task_queue.h
	${CREVENT_SRC_DIR}/task/sequence_manager/task_queue_impl.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/task_queue_impl.h
	${CREVENT_SRC_DIR}/task/sequence_manager/task_queue_selector.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/task_queue_selector.h
	${CREVENT_SRC_DIR}/task/sequence_manager/task_queue_selector_logic.h
	${CREVENT_SRC_DIR}/task/sequence_manager/task_time_observer.h
	${CREVENT_SRC_DIR}/task/sequence_manager/tasks.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/tasks.h
	${CREVENT_SRC_DIR}/task/sequence_manager/thread_controller.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/thread_controller.h
	${CREVENT_SRC_DIR}/task/sequence_manager/thread_controller_impl.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/thread_controller_impl.h
	${CREVENT_SRC_DIR}/task/sequence_manager/thread_controller_with_message_pump_impl.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/thread_controller_with_message_pump_impl.h
	${CREVENT_SRC_DIR}/task/sequence_manager/time_domain.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/time_domain.h
	${CREVENT_SRC_DIR}/task/sequence_manager/work_deduplicator.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/work_deduplicator.h
	${CREVENT_SRC_DIR}/task/sequence_manager/work_queue.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/work_queue.h
	${CREVENT_SRC_DIR}/task/sequence_manager/work_queue_sets.cc
	${CREVENT_SRC_DIR}/task/sequence_manager/work_queue_sets.h
)

# compiler options#
if (${MINI_CHROMIUM_COMPONENT_BUILD})
	add_library(crevent SHARED ${CREVENT_SRC_FILES})
	add_dependencies(crevent crbase)

	# macros
	target_compile_definitions(crevent PRIVATE 
		MINI_CHROMIUM_COMPONENT_BUILD 
		CREVENT_IMPLEMENTATION)

	target_link_libraries(crevent crbase)
else()
	add_library(crevent STATIC ${CREVENT_SRC_FILES})
	add_dependencies(crevent crbase)
endif()

if(${MINI_CHROMIUM_STATIC_CRT})
	if(${MINI_CHROMIUM_COMPILER_MSVC})
		target_compile_options(crevent PRIVATE 
			$<$<CONFIG:>:/MT>
			$<$<CONFIG:Debug>:/MTd>
			$<$<CONFIG:Release>:/MT>
		)
	endif()
endif()

target_include_directories(crevent PRIVATE ${MINI_CHROMIUM_SRC_DIR})

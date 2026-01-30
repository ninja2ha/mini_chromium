// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_TASK_EXECUTOR_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_TASK_EXECUTOR_H_

#include <stdint.h>

#include "crbase/memory/ref_counted.h"

#include "crbase_runtime/runtime_export.h"
#include "crbase_runtime/sequenced_task_runner.h"
#include "crbase_runtime/single_thread_task_runner.h"
#include "crbase_runtime/task/single_thread_task_runner_thread_mode.h"
#include "crbase_runtime/task_runner.h"

#include "crbuild/build_config.h"

namespace cr {

class Location;
class TaskTraits;

// A TaskExecutor can execute Tasks with a specific TaskTraits extension id. To
// handle Tasks posted via the //base/task/post_task.h API, the TaskExecutor
// should be registered by calling RegisterTaskExecutor().
class CRBASE_RT_EXPORT TaskExecutor {
 public:
  virtual ~TaskExecutor() = default;

  // Posts |task| with a |delay| and specific |traits|. |delay| can be zero. For
  // one off tasks that don't require a TaskRunner. Returns false if the task
  // definitely won't run because of current shutdown state.
  virtual bool PostDelayedTask(const Location& from_here,
                               const TaskTraits& traits,
                               OnceClosure task,
                               TimeDelta delay) = 0;

  // Returns a TaskRunner whose PostTask invocations result in scheduling tasks
  // using |traits|. Tasks may run in any order and in parallel.
  virtual RefPtr<TaskRunner> CreateTaskRunner(
      const TaskTraits& traits) = 0;

  // Returns a SequencedTaskRunner whose PostTask invocations result in
  // scheduling tasks using |traits|. Tasks run one at a time in posting order.
  virtual RefPtr<SequencedTaskRunner> CreateSequencedTaskRunner(
      const TaskTraits& traits) = 0;

  // Returns a SingleThreadTaskRunner whose PostTask invocations result in
  // scheduling tasks using |traits|. Tasks run on a single thread in posting
  // order. If |traits| identifies an existing thread,
  // SingleThreadTaskRunnerThreadMode::SHARED must be used.
  virtual RefPtr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) = 0;

#if defined(MINI_CHROMIUM_OS_WIN)
  // Returns a SingleThreadTaskRunner whose PostTask invocations result in
  // scheduling tasks using |traits| in a COM Single-Threaded Apartment. Tasks
  // run in the same Single-Threaded Apartment in posting order for the returned
  // SingleThreadTaskRunner. If |traits| identifies an existing thread,
  // SingleThreadTaskRunnerThreadMode::SHARED must be used.
  virtual RefPtr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) = 0;
#endif  // defined(MINI_CHROMIUM_OS_WIN)
};

// Register a TaskExecutor with the //base/task/post_task.h API in the current
// process for tasks subsequently posted with a TaskTraits extension with the
// given |extension_id|. All executors need to be registered before any tasks
// are posted with |extension_id|. Only one executor per |extension_id| is
// supported.
CRBASE_RT_EXPORT void RegisterTaskExecutor(uint8_t extension_id,
                                           TaskExecutor* task_executor);
CRBASE_RT_EXPORT void UnregisterTaskExecutorForTesting(uint8_t extension_id);

// Stores the provided TaskExecutor in TLS for the current thread, to be used by
// tasks with the CurrentThread() trait.
CRBASE_RT_EXPORT void SetTaskExecutorForCurrentThread(
    TaskExecutor* task_executor);

// Returns the task executor registered for the current thread.
CRBASE_RT_EXPORT TaskExecutor* GetTaskExecutorForCurrentThread();

// Determines whether a registered TaskExecutor will handle tasks with the given
// |traits| and, if so, returns a pointer to it. Otherwise, returns |nullptr|.
CRBASE_RT_EXPORT TaskExecutor* GetRegisteredTaskExecutorForTraits(
    const TaskTraits& traits);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_TASK_EXECUTOR_H_

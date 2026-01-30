// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SIMPLE_TASK_EXECUTOR_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SIMPLE_TASK_EXECUTOR_H_

#include "crbase_runtime/task/task_executor.h"
#include "crbuild/build_config.h"

namespace cr {

// A simple TaskExecutor with exactly one SingleThreadTaskRunner.
// Must be instantiated and destroyed on the thread that runs tasks for the
// SingleThreadTaskRunner.
class CRBASE_RT_EXPORT SimpleTaskExecutor : public TaskExecutor {
 public:
  explicit SimpleTaskExecutor(RefPtr<SingleThreadTaskRunner> task_queue);

  ~SimpleTaskExecutor() override;

  bool PostDelayedTask(const Location& from_here,
                       const TaskTraits& traits,
                       OnceClosure task,
                       TimeDelta delay) override;

  RefPtr<TaskRunner> CreateTaskRunner(const TaskTraits& traits) override;

  RefPtr<SequencedTaskRunner> CreateSequencedTaskRunner(
      const TaskTraits& traits) override;

  RefPtr<SingleThreadTaskRunner> CreateSingleThreadTaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;

#if defined(MINI_CHROMIUM_OS_WIN)
  RefPtr<SingleThreadTaskRunner> CreateCOMSTATaskRunner(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;
#endif  // defined(MINI_CHROMIUM_OS_WIN)

 private:
  const RefPtr<SingleThreadTaskRunner> task_queue_;

  // In tests there may already be a TaskExecutor registered for the thread, we
  // keep tack of the previous TaskExecutor and restored it upon destruction.
  TaskExecutor* const previous_task_executor_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SIMPLE_TASK_EXECUTOR_H_

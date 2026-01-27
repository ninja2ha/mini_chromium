// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/task/simple_task_executor.h"

namespace cr {

SimpleTaskExecutor::SimpleTaskExecutor(
    RefPtr<SingleThreadTaskRunner> task_queue)
    : task_queue_(std::move(task_queue)),
      previous_task_executor_(GetTaskExecutorForCurrentThread()) {
  CR_DCHECK(task_queue_);
  // The TaskExecutor API does not expect nesting, but this can happen in tests
  // so we have to work around it here.
  if (previous_task_executor_)
    SetTaskExecutorForCurrentThread(nullptr);
  SetTaskExecutorForCurrentThread(this);
}

SimpleTaskExecutor::~SimpleTaskExecutor() {
  if (previous_task_executor_)
    SetTaskExecutorForCurrentThread(nullptr);
  SetTaskExecutorForCurrentThread(previous_task_executor_);
}

bool SimpleTaskExecutor::PostDelayedTask(const Location& from_here,
                                         const TaskTraits& traits,
                                         OnceClosure task,
                                         TimeDelta delay) {
  return task_queue_->PostDelayedTask(from_here, std::move(task), delay);
}

RefPtr<TaskRunner> SimpleTaskExecutor::CreateTaskRunner(
    const TaskTraits& traits) {
  return task_queue_;
}

RefPtr<SequencedTaskRunner>
SimpleTaskExecutor::CreateSequencedTaskRunner(const TaskTraits& traits) {
  return task_queue_;
}

RefPtr<SingleThreadTaskRunner>
SimpleTaskExecutor::CreateSingleThreadTaskRunner(
    const TaskTraits& traits,
    SingleThreadTaskRunnerThreadMode thread_mode) {
  return task_queue_;
}

#if defined(MINI_CHROMIUM_OS_WIN)
RefPtr<SingleThreadTaskRunner>
SimpleTaskExecutor::CreateCOMSTATaskRunner(
    const TaskTraits& traits,
    SingleThreadTaskRunnerThreadMode thread_mode) {
  // It seems pretty unlikely this will be used on a comsta task thread.
  CR_NOTREACHED();
  return task_queue_;
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

}  // namespace cr

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/thread_task_runner_handle.h"

#include <memory>
#include <utility>

#include "crbase/memory/no_destructor.h"
#include "crbase/logging.h"
#include "crbase/threading/thread_local.h"

namespace cr {

namespace {

ThreadLocalPointer<ThreadTaskRunnerHandle>* thread_task_runner_tls() {
  static cr::NoDestructor<
      ThreadLocalPointer<ThreadTaskRunnerHandle>> task_runner_tls;
  return task_runner_tls.get();
};

}  // namespace

// static
const RefPtr<SingleThreadTaskRunner>& ThreadTaskRunnerHandle::Get() {
  const ThreadTaskRunnerHandle* current =
      thread_task_runner_tls()->Get();
  CR_CHECK(current)
      << "Error: This caller requires a single-threaded context (i.e. the "
         "current task needs to run from a SingleThreadTaskRunner). If you're "
         "in a test refer to //docs/threading_and_tasks_testing.md.";
  return current->task_runner_;
}

// static
bool ThreadTaskRunnerHandle::IsSet() {
  return !!thread_task_runner_tls()->Get();
}

ThreadTaskRunnerHandle::ThreadTaskRunnerHandle(
    RefPtr<SingleThreadTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)),
      sequenced_task_runner_handle_(task_runner_) {
  CR_DCHECK(task_runner_->BelongsToCurrentThread());
  CR_DCHECK(!thread_task_runner_tls()->Get());
  thread_task_runner_tls()->Set(this);
}

ThreadTaskRunnerHandle::~ThreadTaskRunnerHandle() {
  CR_DCHECK(task_runner_->BelongsToCurrentThread());
  CR_DCHECK(thread_task_runner_tls()->Get() == this);
  thread_task_runner_tls()->Set(nullptr);
}

}  // namespace cr
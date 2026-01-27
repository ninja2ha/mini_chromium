// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/threading/thread_task_runner_handle.h"

#include <memory>
#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/memory/no_destructor.h"
///#include "crbase/lazy_instance.h"
#include "crbase/threading/thread_local.h"
#include "crbase_runtime/run_loop.h"

namespace cr {

namespace {

cr::ThreadLocalPointer<ThreadTaskRunnerHandle>* GetThreadTaskRunnerTls() {
  static cr::NoDestructor<cr::ThreadLocalPointer<ThreadTaskRunnerHandle>> tls;
  return tls.get();
}

}  // namespace

// static
const RefPtr<SingleThreadTaskRunner>& ThreadTaskRunnerHandle::Get() {
  const ThreadTaskRunnerHandle* current = GetThreadTaskRunnerTls()->Get();
  CR_CHECK(current)
      << "Error: This caller requires a single-threaded context (i.e. the "
         "current task needs to run from a SingleThreadTaskRunner). If you're "
         "in a test refer to //docs/threading_and_tasks_testing.md.";
  return current->task_runner_;
}

// static
bool ThreadTaskRunnerHandle::IsSet() {
  return !!GetThreadTaskRunnerTls()->Get();
}

ThreadTaskRunnerHandle::ThreadTaskRunnerHandle(
    RefPtr<SingleThreadTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)),
      sequenced_task_runner_handle_(task_runner_) {
  CR_DCHECK(task_runner_->BelongsToCurrentThread());
  CR_DCHECK(!GetThreadTaskRunnerTls()->Get());
  GetThreadTaskRunnerTls()->Set(this);
}

ThreadTaskRunnerHandle::~ThreadTaskRunnerHandle() {
  CR_DCHECK(task_runner_->BelongsToCurrentThread());
  CR_DCHECK(GetThreadTaskRunnerTls()->Get() == this);
  GetThreadTaskRunnerTls()->Set(nullptr);
}

ThreadTaskRunnerHandleOverride::ThreadTaskRunnerHandleOverride(
    RefPtr<SingleThreadTaskRunner> overriding_task_runner,
    bool allow_nested_runloop) {
  CR_DCHECK(!SequencedTaskRunnerHandle::IsSet() || ThreadTaskRunnerHandle::IsSet())
      << "ThreadTaskRunnerHandleOverride is not compatible with a "
         "SequencedTaskRunnerHandle already being set on this thread (except "
         "when it's set by the current ThreadTaskRunnerHandle).";

  if (!ThreadTaskRunnerHandle::IsSet()) {
    top_level_thread_task_runner_handle_.emplace(
        std::move(overriding_task_runner));
    return;
  }

#if CR_DCHECK_IS_ON()
  expected_task_runner_before_restore_ = overriding_task_runner.get();
#endif
  ThreadTaskRunnerHandle* ttrh = GetThreadTaskRunnerTls()->Get();
  ttrh->sequenced_task_runner_handle_.task_runner_ = overriding_task_runner;
  ttrh->task_runner_.swap(overriding_task_runner);
  // Due to the swap, now `ttrh->task_runner_` points to the overriding task
  // runner and `overriding_task_runner_` points to the previous task runner.
  task_runner_to_restore_ = std::move(overriding_task_runner);

  if (!allow_nested_runloop)
    no_running_during_override_.emplace();
}

ThreadTaskRunnerHandleOverride::~ThreadTaskRunnerHandleOverride() {
  if (task_runner_to_restore_) {
    ThreadTaskRunnerHandle* ttrh = GetThreadTaskRunnerTls()->Get();

#if CR_DCHECK_IS_ON()
    CR_DCHECK(expected_task_runner_before_restore_ == ttrh->task_runner_.get())
        << "Nested overrides must expire their ThreadTaskRunnerHandleOverride "
           "in LIFO order.";
#endif

    ttrh->sequenced_task_runner_handle_.task_runner_ = task_runner_to_restore_;
    ttrh->task_runner_.swap(task_runner_to_restore_);
  }
}

}  // namespace cr
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_

#include "crbase/base_export.h"
#include "crbase/functional/callback_helpers.h"
#include "crbase/compiler_specific.h"
#include "crbase/memory/ref_ptr.h"
#include "crbase/threading/task/single_thread_task_runner.h"
#include "crbase/threading/task/sequenced_task_runner_handle.h"

namespace cr {

// ThreadTaskRunnerHandle stores a reference to a thread's TaskRunner
// in thread-local storage.  Callers can then retrieve the TaskRunner
// for the current thread by calling ThreadTaskRunnerHandle::Get().
// At most one TaskRunner may be bound to each thread at a time.
// Prefer SequencedTaskRunnerHandle to this unless thread affinity is required.
class CRBASE_EXPORT ThreadTaskRunnerHandle {
 public:
  ThreadTaskRunnerHandle(const ThreadTaskRunnerHandle&) = delete;
  ThreadTaskRunnerHandle& operator=(const ThreadTaskRunnerHandle&) = delete;

  // Gets the SingleThreadTaskRunner for the current thread.
  static const RefPtr<SingleThreadTaskRunner>& Get() CR_WARN_UNUSED_RESULT;

  // Returns true if the SingleThreadTaskRunner is already created for
  // the current thread.
  static bool IsSet() CR_WARN_UNUSED_RESULT;

  // Binds |task_runner| to the current thread. |task_runner| must belong
  // to the current thread for this to succeed.
  explicit ThreadTaskRunnerHandle(RefPtr<SingleThreadTaskRunner> task_runner);
  ~ThreadTaskRunnerHandle();

 private:
  RefPtr<SingleThreadTaskRunner> task_runner_;

  // Registers |task_runner_|'s SequencedTaskRunner interface as the
  // SequencedTaskRunnerHandle on this thread.
  SequencedTaskRunnerHandle sequenced_task_runner_handle_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
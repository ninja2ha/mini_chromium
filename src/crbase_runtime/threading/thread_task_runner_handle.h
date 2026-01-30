// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_THREAD_TASK_RUNNER_HANDLE_H_

#include "crbase_runtime/runtime_export.h"
#include "crbase/logging/logging.h"
#include "crbase/memory/ref_ptr.h"
#include "crbase/internal/gtest_prod_util.h"
#include "crbase_runtime/single_thread_task_runner.h"
#include "crbase_runtime/threading/sequenced_task_runner_handle.h"
#include "crbase_runtime/run_loop.h"
#include "crbuild/compiler_specific.h"

namespace cr {

// ThreadTaskRunnerHandle stores a reference to a thread's TaskRunner
// in thread-local storage.  Callers can then retrieve the TaskRunner
// for the current thread by calling ThreadTaskRunnerHandle::Get().
// At most one TaskRunner may be bound to each thread at a time.
// Prefer SequencedTaskRunnerHandle to this unless thread affinity is required.
class CRBASE_RT_EXPORT ThreadTaskRunnerHandle {
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
  explicit ThreadTaskRunnerHandle(
      RefPtr<SingleThreadTaskRunner> task_runner);
  ~ThreadTaskRunnerHandle();

 private:
  friend class ThreadTaskRunnerHandleOverride;
  RefPtr<SingleThreadTaskRunner> task_runner_;

  // Registers |task_runner_|'s SequencedTaskRunner interface as the
  // SequencedTaskRunnerHandle on this thread.
  SequencedTaskRunnerHandle sequenced_task_runner_handle_;
};

// ThreadTaskRunnerHandleOverride overrides the task runner returned by
// |ThreadTaskRunnerHandle::Get()| to point at |overriding_task_runner| until
// the |ThreadTaskRunnerHandleOverride| goes out of scope.
// ThreadTaskRunnerHandleOverride instantiates a new ThreadTaskRunnerHandle if
// ThreadTaskRunnerHandle is not instantiated on the current thread. Nested
// overrides are allowed but callers must ensure the
// |ThreadTaskRunnerHandleOverride| expire in LIFO (stack) order.
//
// Note: nesting ThreadTaskRunnerHandle is subtle and should be done with care,
// hence the need to friend and request a //base/OWNERS review for usage outside
// of tests. Use ThreadTaskRunnerHandleOverrideForTesting to bypass the friend
// requirement in tests.
class CRBASE_RT_EXPORT ThreadTaskRunnerHandleOverride {
 public:
  ThreadTaskRunnerHandleOverride(const ThreadTaskRunnerHandleOverride&) =
      delete;
  ThreadTaskRunnerHandleOverride& operator=(
      const ThreadTaskRunnerHandleOverride&) = delete;
  ~ThreadTaskRunnerHandleOverride();

 private:
  friend class ThreadTaskRunnerHandleOverrideForTesting;
  CR_FRIEND_GTEST_ALL_PREFIXES(ThreadTaskRunnerHandleTest, NestedRunLoop);

  // We expect ThreadTaskRunnerHandleOverride to be only needed under special
  // circumstances. Require them to be enumerated as friends to require
  // //base/OWNERS review. Use ThreadTaskRunnerHandleOverrideForTesting
  // in unit tests to avoid the friend requirement.

  // Constructs a ThreadTaskRunnerHandleOverride which will make
  // ThreadTaskRunnerHandle::Get() return |overriding_task_runner| for its
  // lifetime. |allow_nested_loop| specifies whether RunLoop::Run() is allowed
  // during this override's lifetime. It's not recommended to allow this unless
  // the current thread's scheduler guarantees that only tasks which pertain to
  // |overriding_task_runner|'s context will be run by nested RunLoops.
  explicit ThreadTaskRunnerHandleOverride(
      RefPtr<SingleThreadTaskRunner> overriding_task_runner,
      bool allow_nested_runloop = false);

  cr::Optional<ThreadTaskRunnerHandle> top_level_thread_task_runner_handle_;
  RefPtr<SingleThreadTaskRunner> task_runner_to_restore_;
#if CR_DCHECK_IS_ON()
  SingleThreadTaskRunner* expected_task_runner_before_restore_{nullptr};
#endif
  cr::Optional<RunLoop::ScopedDisallowRunning> no_running_during_override_;
};

// Note: nesting ThreadTaskRunnerHandles isn't generally desired but it's useful
// in some unit tests where multiple task runners share the main thread for
// simplicity and determinism. Only use this when no other constructs will work
// (see base/test/task_environment.h and base/test/test_mock_time_task_runner.h
// for preferred alternatives).
class ThreadTaskRunnerHandleOverrideForTesting {
 public:
  explicit ThreadTaskRunnerHandleOverrideForTesting(
      RefPtr<SingleThreadTaskRunner> overriding_task_runner)
      : thread_task_runner_handle_override_(std::move(overriding_task_runner)) {
  }

 private:
  ThreadTaskRunnerHandleOverride thread_task_runner_handle_override_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
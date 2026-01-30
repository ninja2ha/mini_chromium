// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_

#include "crbase_runtime/runtime_export.h"
#include "crbase/memory/ref_ptr.h"
#include "crbase_runtime/sequenced_task_runner.h"
#include "crbuild/compiler_specific.h"

namespace cr {

class ThreadTaskRunnerHandle;

class CRBASE_RT_EXPORT SequencedTaskRunnerHandle {
 public:
  SequencedTaskRunnerHandle(const SequencedTaskRunnerHandle&) = delete;
  SequencedTaskRunnerHandle& operator=(const SequencedTaskRunnerHandle&) 
      = delete;

  // Returns a SequencedTaskRunner which guarantees that posted tasks will only
  // run after the current task is finished and will satisfy a SequenceChecker.
  // It should only be called if IsSet() returns true (see the comment there for
  // the requirements).
  static const RefPtr<SequencedTaskRunner>& Get() CR_WARN_UNUSED_RESULT;

  // Returns true if one of the following conditions is fulfilled:
  // a) A SequencedTaskRunner has been assigned to the current thread by
  //    instantiating a SequencedTaskRunnerHandle.
  // b) The current thread has a ThreadTaskRunnerHandle (which includes any
  //    thread that has a MessageLoop associated with it).
  static bool IsSet() CR_WARN_UNUSED_RESULT;

  // Binds |task_runner| to the current thread.
  explicit SequencedTaskRunnerHandle(
      RefPtr<SequencedTaskRunner> task_runner);
  ~SequencedTaskRunnerHandle();

 private:
  friend class ThreadTaskRunnerHandleOverride;

  RefPtr<SequencedTaskRunner> task_runner_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_
// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_COMMON_SCOPED_DEFER_TASK_POSTING_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_COMMON_SCOPED_DEFER_TASK_POSTING_H_

#include <vector>

#include "crbase/base_export.h"
#include "crbase/location.h"
#include "crbase_runtime/sequenced_task_runner.h"

namespace cr {

// Tracing wants to post tasks from within a trace event within PostTask, but
// this can lead to a deadlock. Create a scope to ensure that we are posting
// the tasks in question outside of the scope of the lock.
// NOTE: This scope affects only the thread it is created on. All other threads
// still can post tasks.
//
// TODO(altimin): It should be possible to get rid of this scope, but this
// requires refactoring TimeDomain to ensure that TimeDomain never changes and
// we can read current time without grabbing a lock.
class CRBASE_EXPORT ScopedDeferTaskPosting {
 public:
  static void PostOrDefer(RefPtr<SequencedTaskRunner> task_runner,
                          const Location& from_here,
                          OnceClosure task,
                          cr::TimeDelta delay);

  static bool IsPresent();

  ScopedDeferTaskPosting(const ScopedDeferTaskPosting&) = delete;
  ScopedDeferTaskPosting& operator=(const ScopedDeferTaskPosting&) = delete;

  ScopedDeferTaskPosting();
  ~ScopedDeferTaskPosting();

 private:
  static ScopedDeferTaskPosting* Get();
  // Returns whether the |scope| was set as active, which happens only
  // when the scope wasn't set before.
  static bool Set(ScopedDeferTaskPosting* scope);

  void DeferTaskPosting(RefPtr<SequencedTaskRunner> task_runner,
                        const Location& from_here,
                        OnceClosure task,
                        cr::TimeDelta delay);

  struct DeferredTask {
    DeferredTask(const DeferredTask&) = delete;
    DeferredTask& operator=(const DeferredTask&) = delete;

    DeferredTask(RefPtr<SequencedTaskRunner> task_runner,
                 Location from_here,
                 OnceClosure task,
                 cr::TimeDelta delay);
    DeferredTask(DeferredTask&& task);
    ~DeferredTask();

    RefPtr<SequencedTaskRunner> task_runner;
    Location from_here;
    OnceClosure task;
    cr::TimeDelta delay;
  };

  std::vector<DeferredTask> deferred_tasks_;

  // Scopes can be nested (e.g. ScheduleWork inside PostTasks can post a task
  // to another task runner), so we want to know whether the scope is top-level
  // or not.
  bool top_level_scope_ = false;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_COMMON_SCOPED_DEFER_TASK_POSTING_H_

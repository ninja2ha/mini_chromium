// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_SEQUENCED_TASK_SOURCE_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_SEQUENCED_TASK_SOURCE_H_

#include "crbase/containers/optional.h"
#include "crbase_runtime/pending_task.h"
#include "crbase_runtime/task/sequence_manager/lazy_now.h"
#include "crbase_runtime/task/sequence_manager/tasks.h"

namespace cr {
namespace sequence_manager {
namespace internal {

// Interface to pass tasks to ThreadController.
class CRBASE_RT_EXPORT SequencedTaskSource {
 public:
  enum class SelectTaskOption { kDefault, kSkipDelayedTask };

  virtual ~SequencedTaskSource() = default;

  // Returns the next task to run from this source or nullptr if
  // there're no more tasks ready to run. If a task is returned,
  // DidRunTask() must be invoked before the next call to SelectNextTask().
  // |option| allows control on which kind of tasks can be selected.
  virtual Task* SelectNextTask(
      SelectTaskOption option = SelectTaskOption::kDefault) = 0;

  // Notifies this source that the task previously obtained
  // from SelectNextTask() has been completed.
  virtual void DidRunTask() = 0;

  // Returns the delay till the next task or TimeDelta::Max()
  // if there are no tasks left. |option| allows control on which kind of tasks
  // can be selected.
  virtual TimeDelta DelayTillNextTask(
      LazyNow* lazy_now,
      SelectTaskOption option = SelectTaskOption::kDefault) const = 0;

  // Return true if there are any pending tasks in the task source which require
  // high resolution timing.
  virtual bool HasPendingHighResolutionTasks() = 0;

  // Called when we have run out of immediate work.  If more immediate work
  // becomes available as a result of any processing done by this callback,
  // return true to schedule a future DoWork.
  virtual bool OnSystemIdle() = 0;
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_SEQUENCED_TASK_SOURCE_H_

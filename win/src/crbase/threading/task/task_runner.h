// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_TASK_RUNNER_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_TASK_RUNNER_H_

#include <stddef.h>

#include "crbase/base_export.h"
#include "crbase/functional/callback.h"
#include "crbase/location.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/time/time.h"

namespace cr {

struct TaskRunnerTraits;

// A TaskRunner is an object that runs posted tasks (in the form of
// OnceClosure objects).  The TaskRunner interface provides a way of
// decoupling task posting from the mechanics of how each task will be
// run.  TaskRunner provides very weak guarantees as to how posted
// tasks are run (or if they're run at all).  In particular, it only
// guarantees:
//
//   - Posting a task will not run it synchronously.  That is, no
//     Post*Task method will call task.Run() directly.
//
//   - Increasing the delay can only delay when the task gets run.
//     That is, increasing the delay may not affect when the task gets
//     run, or it could make it run later than it normally would, but
//     it won't make it run earlier than it normally would.
//
// TaskRunner does not guarantee the order in which posted tasks are
// run, whether tasks overlap, or whether they're run on a particular
// thread.  Also it does not guarantee a memory model for shared data
// between tasks.  (In other words, you should use your own
// synchronization/locking primitives if you need to share data
// between tasks.)
//
// Implementations of TaskRunner should be thread-safe in that all
// methods must be safe to call on any thread.  Ownership semantics
// for TaskRunners are in general not clear, which is why the
// interface itself is RefCountedThreadSafe.
//
// Some theoretical implementations of TaskRunner:
//
//   - A TaskRunner that uses a thread pool to run posted tasks.
//
//   - A TaskRunner that, for each task, spawns a non-joinable thread
//     to run that task and immediately quit.
//
//   - A TaskRunner that stores the list of posted tasks and has a
//     method Run() that runs each runnable task in random order.
class CRBASE_EXPORT TaskRunner
    : public RefCountedThreadSafe<TaskRunner, TaskRunnerTraits> {
 public:
  // Posts the given task to be run.  Returns true if the task may be
  // run at some point in the future, and false if the task definitely
  // will not be run.
  //
  // Equivalent to PostDelayedTask(from_here, task, 0).
  bool PostTask(const Location& from_here, OnceClosure task);

  // Like PostTask, but tries to run the posted task only after |delay_ms|
  // has passed. Implementations should use a tick clock, rather than wall-
  // clock time, to implement |delay|.
  virtual bool PostDelayedTask(const Location& from_here,
                               OnceClosure task,
                               cr::TimeDelta delay) = 0;

  // Drepecated: favor RunsTasksInCurrentSequence().
  // TODO(http://crbug.com/665062): mass redirect callers and remove this.
  bool RunsTasksOnCurrentThread() const {
    return RunsTasksInCurrentSequence();
  }

  // Returns true iff tasks posted to this TaskRunner are sequenced
  // with this call.
  //
  // In particular:
  // - Returns true if this is a SequencedTaskRunner to which the
  //   current task was posted.
  // - Returns true if this is a SequencedTaskRunner bound to the
  //   same sequence as the SequencedTaskRunner to which the current
  //   task was posted.
  // - Returns true if this is a SingleThreadTaskRunner bound to
  //   the current thread.
  // TODO(http://crbug.com/665062):
  //   This API doesn't make sense for parallel TaskRunners.
  //   Introduce alternate static APIs for documentation purposes of "this runs
  //   in pool X", have RunsTasksInCurrentSequence() return false for parallel
  //   TaskRunners, and ultimately move this method down to SequencedTaskRunner.
  virtual bool RunsTasksInCurrentSequence() const = 0;

  // Posts |task| on the current TaskRunner.  On completion, |reply|
  // is posted to the thread that called PostTaskAndReply().  Both
  // |task| and |reply| are guaranteed to be deleted on the thread
  // from which PostTaskAndReply() is invoked.  This allows objects
  // that must be deleted on the originating thread to be bound into
  // the |task| and |reply| OnceClosures.  In particular, it can be useful
  // to use WeakPtr<> in the |reply| OnceClosure so that the reply
  // operation can be canceled. See the following pseudo-code:
  //
  // class DataBuffer : public RefCountedThreadSafe<DataBuffer> {
  //  public:
  //   // Called to add data into a buffer.
  //   void AddData(void* buf, size_t length);
  //   ...
  // };
  //
  //
  // class DataLoader : public SupportsWeakPtr<DataLoader> {
  //  public:
  //    void GetData() {
  //      cr::RefPtr<DataBuffer> buffer = new DataBuffer();
  //      target_thread_.task_runner()->PostTaskAndReply(
  //          CR_FROM_HERE,
  //          cr::BindOnce(&DataBuffer::AddData, buffer),
  //          cr::BindOnce(&DataLoader::OnDataReceived, AsWeakPtr(), buffer));
  //    }
  //
  //  private:
  //    void OnDataReceived(cr::RefPtr<DataBuffer> buffer) {
  //      // Do something with buffer.
  //    }
  // };
  //
  //
  // Things to notice:
  //   * Results of |task| are shared with |reply| by binding a shared argument
  //     (a DataBuffer instance).
  //   * The DataLoader object has no special thread safety.
  //   * The DataLoader object can be deleted while |task| is still running,
  //     and the reply will cancel itself safely because it is bound to a
  //     WeakPtr<>.
  bool PostTaskAndReply(const Location& from_here,
                        OnceClosure task,
                        OnceClosure reply);

 protected:
  friend struct TaskRunnerTraits;

  TaskRunner();
  virtual ~TaskRunner();

  // Called when this object should be destroyed.  By default simply
  // deletes |this|, but can be overridden to do something else, like
  // delete on a certain thread.
  virtual void OnDestruct() const;
};

struct CRBASE_EXPORT TaskRunnerTraits {
  static void Destruct(const TaskRunner* task_runner);
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_TASK_RUNNER_H_
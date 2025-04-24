// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H_
#define MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H_

#include "crbase/base_export.h"
#include "crbase/functional/callback.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/message_loop/pending_task.h"
#include "crbase/synchronization/lock.h"
#include "crbase/time/time.h"

namespace cr {

class MessageLoop;

namespace internal {

// Implements a queue of tasks posted to the message loop running on the current
// thread. This class takes care of synchronizing posting tasks from different
// threads and together with MessageLoop ensures clean shutdown.
class CRBASE_EXPORT IncomingTaskQueue
    : public RefCountedThreadSafe<IncomingTaskQueue> {
 public:
  IncomingTaskQueue(const IncomingTaskQueue&) = delete;
  IncomingTaskQueue& operator=(const IncomingTaskQueue&) = delete;

  explicit IncomingTaskQueue(MessageLoop* message_loop);

  // Appends a task to the incoming queue. Posting of all tasks is routed though
  // AddToIncomingQueue() or TryAddToIncomingQueue() to make sure that posting
  // task is properly synchronized between different threads.
  //
  // Returns true if the task was successfully added to the queue, otherwise
  // returns false. In all cases, the ownership of |task| is transferred to the
  // called method.
  bool AddToIncomingQueue(const cr::Location& from_here,
                          OnceClosure task,
                          TimeDelta delay,
                          bool nestable);

  // Returns true if the queue contains tasks that require higher than default
  // timer resolution. Currently only needed for Windows.
  bool HasHighResolutionTasks();

  // Loads tasks from the |incoming_queue_| into |*work_queue|. Must be called
  // from the thread that is running the loop. Returns the number of tasks that
  // require high resolution timers.
  int ReloadWorkQueue(TaskQueue* work_queue);

  // Disconnects |this| from the parent message loop.
  void WillDestroyCurrentMessageLoop();

  // This should be called when the message loop becomes ready for
  // scheduling work.
  void StartScheduling();

 private:
  friend class RefCountedThreadSafe<IncomingTaskQueue>;
  virtual ~IncomingTaskQueue();

  // Calculates the time at which a PendingTask should run.
  TimeTicks CalculateDelayedRuntime(TimeDelta delay);

  // Adds a task to |incoming_queue_|. The caller retains ownership of
  // |pending_task|, but this function will reset the value of
  // |pending_task->task|. This is needed to ensure that the posting call stack
  // does not retain |pending_task->task| beyond this function call.
  bool PostPendingTask(PendingTask* pending_task);

  // Wakes up the message loop and schedules work.
  void ScheduleWork();

  // Number of tasks that require high resolution timing. This value is kept
  // so that ReloadWorkQueue() completes in constant time.
  int high_res_task_count_;

  // The lock that protects access to the members of this class, except
  // |message_loop_|.
  cr::Lock incoming_queue_lock_;

  // An incoming queue of tasks that are acquired under a mutex for processing
  // on this instance's thread. These tasks have not yet been been pushed to
  // |message_loop_|.
  TaskQueue incoming_queue_;

  // Points to the message loop that owns |this|.
  MessageLoop* message_loop_;

  // The next sequence number to use for delayed tasks.
  int next_sequence_num_;

  // True if our message loop has already been scheduled and does not need to be
  // scheduled again until an empty reload occurs.
  bool message_loop_scheduled_;

  // True if we always need to call ScheduleWork when receiving a new task, even
  // if the incoming queue was not empty.
  const bool always_schedule_work_;

  // False until StartScheduling() is called.
  bool is_ready_for_scheduling_;
};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_INCOMING_TASK_QUEUE_H_

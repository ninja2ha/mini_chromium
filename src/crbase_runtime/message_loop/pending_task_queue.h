// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 71.0.3578.141

#ifndef MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_PENDING_TASK_QUEUE_H_
#define MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_PENDING_TASK_QUEUE_H_

#include "crbase/threading/sequence_checker.h"
#include "crbase_runtime/pending_task.h"

namespace cr {
namespace internal {

// Provides storage for tasks deferred by MessageLoop via DelayedQueue and
// DeferredQueue.
class PendingTaskQueue {
 public:
  PendingTaskQueue(const PendingTaskQueue&) = delete;
  PendingTaskQueue& operator=(const PendingTaskQueue&) = delete;

  // Provides a read-write task queue.
  class Queue {
   public:
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue() = default;
    virtual ~Queue() = default;

    // Returns the next task. HasTasks() is assumed to be true.
    virtual const PendingTask& Peek() = 0;

    // Removes and returns the next task. HasTasks() is assumed to be true.
    virtual PendingTask Pop() = 0;

    // Whether this queue has tasks.
    virtual bool HasTasks() = 0;

    // Removes all tasks.
    virtual void Clear() = 0;

    // Adds the task to the end of the queue.
    virtual void Push(PendingTask pending_task) = 0;
  };

  PendingTaskQueue();
  ~PendingTaskQueue();

  Queue& delayed_tasks() { return delayed_tasks_; }

  Queue& deferred_tasks() { return deferred_tasks_; }

  bool HasPendingHighResolutionTasks() const {
    return delayed_tasks_.HasPendingHighResolutionTasks();
  }

  // Reports UMA metrics about its queues before the MessageLoop goes to sleep
  // per being idle.
  void ReportMetricsOnIdle() const;

 private:
  // The queue for holding tasks that should be run later and sorted by expected
  // run time.
  class DelayedQueue : public Queue {
   public:
    DelayedQueue(const DelayedQueue&) = delete;
    DelayedQueue& operator=(const DelayedQueue&) = delete;

    DelayedQueue();
    ~DelayedQueue() override;

    // Queue:
    const PendingTask& Peek() override;
    PendingTask Pop() override;
    // Whether this queue has tasks after sweeping the cancelled ones in front.
    bool HasTasks() override;
    void Clear() override;
    void Push(PendingTask pending_task) override;

    size_t Size() const;
    bool HasPendingHighResolutionTasks() const {
      CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
      return pending_high_res_tasks_ > 0;
    }

   private:
    DelayedTaskQueue queue_;

    // Number of high resolution tasks in |queue_|.
    int pending_high_res_tasks_ = 0;

    CR_SEQUENCE_CHECKER(sequence_checker_);
  };

  // The queue for holding tasks that couldn't be run while the MessageLoop was
  // nested. These are generally processed during the idle stage.
  class DeferredQueue : public Queue {
   public:
    DeferredQueue(const DeferredQueue&) = delete;
    DeferredQueue& operator=(const DeferredQueue&) = delete;

    DeferredQueue();
    ~DeferredQueue() override;

    // Queue:
    const PendingTask& Peek() override;
    PendingTask Pop() override;
    bool HasTasks() override;
    void Clear() override;
    void Push(PendingTask pending_task) override;

   private:
    TaskQueue queue_;

    CR_SEQUENCE_CHECKER(sequence_checker_);
  };

  DelayedQueue delayed_tasks_;
  DeferredQueue deferred_tasks_;
};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_PENDING_TASK_QUEUE_H_
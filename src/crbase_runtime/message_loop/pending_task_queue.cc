// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 71.0.3578.141

#include "crbase_runtime/message_loop/pending_task_queue.h"

#include <utility>

#include "crbase/logging/logging.h"
#include "crbuild/build_config.h"

namespace cr {
namespace internal {

PendingTaskQueue::PendingTaskQueue() = default;

PendingTaskQueue::~PendingTaskQueue() = default;

void PendingTaskQueue::ReportMetricsOnIdle() const {
  ///UMA_HISTOGRAM_COUNTS_1M(
  ///    "MessageLoop.DelayedTaskQueueForUI.PendingTasksCountOnIdle",
  ///    delayed_tasks_.Size());
}

PendingTaskQueue::DelayedQueue::DelayedQueue() {
  // The constructing sequence is not necessarily the running sequence, e.g. in
  // the case of a MessageLoop created unbound.
  CR_DETACH_FROM_SEQUENCE(sequence_checker_);
}

PendingTaskQueue::DelayedQueue::~DelayedQueue() = default;

void PendingTaskQueue::DelayedQueue::Push(PendingTask pending_task) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (pending_task.is_high_res)
    ++pending_high_res_tasks_;

  queue_.push(std::move(pending_task));
}

const PendingTask& PendingTaskQueue::DelayedQueue::Peek() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(!queue_.empty());
  return queue_.top();
}

PendingTask PendingTaskQueue::DelayedQueue::Pop() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(!queue_.empty());
  PendingTask delayed_task = std::move(const_cast<PendingTask&>(queue_.top()));
  queue_.pop();

  if (delayed_task.is_high_res)
    --pending_high_res_tasks_;
  CR_DCHECK(pending_high_res_tasks_ >= 0);

  return delayed_task;
}

bool PendingTaskQueue::DelayedQueue::HasTasks() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  // TODO(robliao): The other queues don't check for IsCancelled(). Should they?
  while (!queue_.empty() && Peek().task.IsCancelled())
    Pop();

  return !queue_.empty();
}

void PendingTaskQueue::DelayedQueue::Clear() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  while (!queue_.empty())
    Pop();
}

size_t PendingTaskQueue::DelayedQueue::Size() const {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return queue_.size();
}

PendingTaskQueue::DeferredQueue::DeferredQueue() {
  // The constructing sequence is not necessarily the running sequence, e.g. in
  // the case of a MessageLoop created unbound.
  CR_DETACH_FROM_SEQUENCE(sequence_checker_);
}

PendingTaskQueue::DeferredQueue::~DeferredQueue() = default;

void PendingTaskQueue::DeferredQueue::Push(PendingTask pending_task) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  queue_.push(std::move(pending_task));
}

const PendingTask& PendingTaskQueue::DeferredQueue::Peek() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(!queue_.empty());
  return queue_.front();
}

PendingTask PendingTaskQueue::DeferredQueue::Pop() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(!queue_.empty());
  PendingTask deferred_task = std::move(queue_.front());
  queue_.pop();
  return deferred_task;
}

bool PendingTaskQueue::DeferredQueue::HasTasks() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return !queue_.empty();
}

void PendingTaskQueue::DeferredQueue::Clear() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  while (!queue_.empty())
    Pop();
}

}  // namespace internal
}  // namespace cr
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/message_loop/message_loop_task_runner.h"

#include <utility>

#include "crbase/location.h"
#include "crbase/logging.h"
#include "crbase/message_loop/incoming_task_queue.h"

namespace cr {
namespace internal {

MessageLoopTaskRunner::MessageLoopTaskRunner(
    RefPtr<IncomingTaskQueue> incoming_queue)
    : incoming_queue_(incoming_queue), valid_thread_id_(kInvalidThreadId) {
}

void MessageLoopTaskRunner::BindToCurrentThread() {
  AutoLock lock(valid_thread_id_lock_);
  CR_DCHECK(kInvalidThreadId == valid_thread_id_);
  valid_thread_id_ = PlatformThread::CurrentId();
}

bool MessageLoopTaskRunner::PostDelayedTask(
    const cr::Location& from_here,
    OnceClosure task,
    cr::TimeDelta delay) {
  CR_DCHECK(!task.is_null()) << from_here.ToString();
  return incoming_queue_->AddToIncomingQueue(from_here, std::move(task), delay,
                                             true);
}

bool MessageLoopTaskRunner::PostNonNestableDelayedTask(
    const cr::Location& from_here,
    OnceClosure task,
    cr::TimeDelta delay) {
  CR_DCHECK(!task.is_null()) << from_here.ToString();
  return incoming_queue_->AddToIncomingQueue(from_here, std::move(task), delay,
                                             false);
}

bool MessageLoopTaskRunner::RunsTasksInCurrentSequence() const {
  AutoLock lock(valid_thread_id_lock_);
  return valid_thread_id_ == PlatformThread::CurrentId();
}

MessageLoopTaskRunner::~MessageLoopTaskRunner() {
}

}  // namespace internal

}  // namespace cr

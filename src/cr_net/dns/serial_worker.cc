// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/dns/serial_worker.h"

#include "cr_base/logging/logging.h"
#include "cr_base/functional/bind.h"
#include "cr_base/location.h"
///#include "base/task/post_task.h"
///#include "base/task/thread_pool.h"
#include "cr_event/task/sequenced_task_runner_handle.h"
#include "cr_event/threading/thread_task_runner_handle.h"

namespace cr {
namespace net {

SerialWorker::SerialWorker()
    : cr::RefCountedDeleteOnSequence<SerialWorker>(
          cr::SequencedTaskRunnerHandle::Get()),
      state_(IDLE) {}

SerialWorker::~SerialWorker() = default;

void SerialWorker::WorkNow() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  switch (state_) {
    case IDLE:
      // We are posting weak pointer to OnWorkJobFinished to avoid leak when
      // PostTaskAndReply fails to post task back to the original
      // task runner. In this case the callback is not destroyed, and the
      // weak reference allows SerialWorker instance to be deleted.
      ///base::ThreadPool::PostTaskAndReply(
      ///    FROM_HERE,
      ///    {base::MayBlock(), base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      ///    base::BindOnce(&SerialWorker::DoWork, this),
      ///    base::BindOnce(&SerialWorker::OnWorkJobFinished,
      ///                   weak_factory_.GetWeakPtr()));
      state_ = WORKING;
      return;
    case WORKING:
      // Remember to re-read after |DoRead| finishes.
      state_ = PENDING;
      return;
    case CANCELLED:
    case PENDING:
      return;
    default:
      CR_NOTREACHED() << "Unexpected state " << state_;
  }
}

void SerialWorker::Cancel() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  state_ = CANCELLED;
}

void SerialWorker::OnWorkJobFinished() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  switch (state_) {
    case CANCELLED:
      return;
    case WORKING:
      state_ = IDLE;
      this->OnWorkFinished();
      return;
    case PENDING:
      state_ = IDLE;
      WorkNow();
      return;
    default:
      CR_NOTREACHED() << "Unexpected state " << state_;
  }
}

}  // namespace net
}  // namespace cr

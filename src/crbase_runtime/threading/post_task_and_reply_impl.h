// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

// This file contains the implementation for TaskRunner::PostTaskAndReply.

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_POST_TASK_AND_REPLY_IMPL_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_POST_TASK_AND_REPLY_IMPL_H_

#include "crbase/functional/callback.h"
#include "crbase/location.h"

#include "crbase_runtime/runtime_export.h"

namespace cr {
namespace internal {

// Inherit from this in a class that implements PostTask to send a task to a
// custom execution context.
//
// If you're looking for a concrete implementation of PostTaskAndReply, you
// probably want base::TaskRunner or base/task/post_task.h
class CRBASE_RT_EXPORT PostTaskAndReplyImpl {
 public:
  virtual ~PostTaskAndReplyImpl() = default;

  // Posts |task| by calling PostTask(). On completion, posts |reply| to the
  // origin sequence. Can only be called when
  // SequencedTaskRunnerHandle::IsSet(). Each callback is deleted synchronously
  // after running, or scheduled for asynchronous deletion on the origin
  // sequence if it can't run (e.g. if a TaskRunner skips it on shutdown). See
  // SequencedTaskRunner::DeleteSoon() for when objects scheduled for
  // asynchronous deletion can be leaked. Note: All //base task posting APIs
  // require callbacks to support deletion on the posting sequence if they can't
  // be scheduled.
  bool PostTaskAndReply(const Location& from_here,
                        OnceClosure task,
                        OnceClosure reply);

 private:
  virtual bool PostTask(const Location& from_here, OnceClosure task) = 0;
};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_THREADING_POST_TASK_AND_REPLY_IMPL_H_
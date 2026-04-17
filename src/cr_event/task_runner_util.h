// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CREVENT_TASK_RUNNER_UTIL_H_
#define MINI_CHROMIUM_SRC_CREVENT_TASK_RUNNER_UTIL_H_

#include <memory>
#include <utility>

#include "cr_base/functional/bind.h"
#include "cr_base/functional/callback.h"
#include "cr_base/logging/logging.h"
#include "cr_event/internal/post_task_and_reply_with_result_internal.h"
#include "cr_event/task_runner.h"

namespace cr {

// When you have these methods
//
//   R DoWorkAndReturn();
//   void Callback(const R& result);
//
// and want to call them in a PostTaskAndReply kind of fashion where the
// result of DoWorkAndReturn is passed to the Callback, you can use
// PostTaskAndReplyWithResult as in this example:
//
// PostTaskAndReplyWithResult(
//     target_thread_.task_runner(),
//     FROM_HERE,
//     BindOnce(&DoWorkAndReturn),
//     BindOnce(&Callback));
//
// DEPRECATED: Prefer calling|task_runner->PostTaskAndReplyWithResult(...)|
// directly.
// TODO(gab): Mass-migrate to the member method.
template <typename TaskReturnType, typename ReplyArgType>
bool PostTaskAndReplyWithResult(TaskRunner* task_runner,
                                const Location& from_here,
                                OnceCallback<TaskReturnType()> task,
                                OnceCallback<void(ReplyArgType)> reply) {
  CR_DCHECK(task);
  CR_DCHECK(reply);
  // std::unique_ptr used to avoid the need of a default constructor.
  auto* result = new std::unique_ptr<TaskReturnType>();
  return task_runner->PostTaskAndReply(
      from_here,
      BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>, std::move(task),
               result),
      BindOnce(&internal::ReplyAdapter<TaskReturnType, ReplyArgType>,
               std::move(reply), Owned(result)));
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CREVENT_TASK_RUNNER_UTIL_H_
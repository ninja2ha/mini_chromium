// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_TASK_RUNNER_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_TASK_RUNNER_UTIL_H_

#include <utility>

#include "crbase/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/bind_helpers.h"
#include "crbase/functional/callback.h"
#include "crbase/threading/task/post_task_and_reply_with_result_internal.h"
#include "crbase/threading/task/task_runner.h"

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
//     CR_FROM_HERE,
//     BindOnce(&DoWorkAndReturn),
//     BindOnce(&Callback));
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

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_TASK_RUNNER_UTIL_H_
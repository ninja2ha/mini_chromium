// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_TASK_OBSERVER_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_TASK_OBSERVER_H_

#include "crbase_runtime/runtime_export.h"
#include "crbase_runtime/pending_task.h"

namespace cr {

// A TaskObserver is an object that receives notifications about tasks being
// processed on the thread it's associated with.
//
// NOTE: A TaskObserver implementation should be extremely fast!

class CRBASE_RT_EXPORT TaskObserver {
 public:
  // This method is called before processing a task.
  // |was_blocked_or_low_priority| indicates if the task was at some point in a
  // queue that was blocked or less important than "normal".
  virtual void WillProcessTask(const PendingTask& pending_task,
                               bool was_blocked_or_low_priority) = 0;

  // This method is called after processing a task.
  virtual void DidProcessTask(const PendingTask& pending_task) = 0;

 protected:
  virtual ~TaskObserver() = default;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_TASK_OBSERVER_H_

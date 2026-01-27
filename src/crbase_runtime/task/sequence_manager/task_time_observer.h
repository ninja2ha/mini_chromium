// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_TASK_TIME_OBSERVER_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_TASK_TIME_OBSERVER_H_

#include "crbase/time/time.h"

namespace cr {
namespace sequence_manager {

// TaskTimeObserver provides an API for observing completion of tasks.
class TaskTimeObserver {
 public:
  TaskTimeObserver() = default;
  TaskTimeObserver(const TaskTimeObserver&) = delete;
  TaskTimeObserver& operator=(const TaskTimeObserver&) = delete;
  virtual ~TaskTimeObserver() = default;

  // To be called when task is about to start.
  virtual void WillProcessTask(TimeTicks start_time) = 0;

  // To be called when task is completed.
  virtual void DidProcessTask(TimeTicks start_time, TimeTicks end_time) = 0;
};

}  // namespace sequence_manager
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_TASK_TIME_OBSERVER_H_

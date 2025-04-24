// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_PENDING_TASK_H_
#define MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_PENDING_TASK_H_

#include <array>
#include <queue>

#include "crbase/base_export.h"
#include "crbase/functional/callback.h"
#include "crbase/location.h"
#include "crbase/time/time.h"

namespace cr {

// Contains data about a pending task. Stored in TaskQueue and DelayedTaskQueue
// for use by classes that queue and execute tasks.
struct CRBASE_EXPORT PendingTask /*: public TrackingInfo*/ {
  PendingTask(const cr::Location& posted_from, OnceClosure task);
  PendingTask(const cr::Location& posted_from,
              OnceClosure task,
              TimeTicks delayed_run_time,
              bool nestable);
  PendingTask(PendingTask&& other);
  ~PendingTask();

  PendingTask& operator=(PendingTask&& other);

  // Used to support sorting.
  bool operator<(const PendingTask& other) const;

  // The task to run.
  OnceClosure task;

  // The site this PendingTask was posted from.
  cr::Location posted_from;

  // Task backtrace.
  std::array<const void*, 4> task_backtrace;

  // Secondary sort key for run time.
  int sequence_num;

  // OK to dispatch from a nested loop.
  bool nestable;

  // Needs high resolution timers.
  bool is_high_res;

  // The time when the task should be run.
  cr::TimeTicks delayed_run_time;
};

using TaskQueue = std::queue<PendingTask>;

// PendingTasks are sorted by their |delayed_run_time| property.
using DelayedTaskQueue = std::priority_queue<cr::PendingTask>;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_PENDING_TASK_H_

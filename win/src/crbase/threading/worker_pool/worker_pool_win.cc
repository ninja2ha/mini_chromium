// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/worker_pool/worker_pool.h"

#include <utility>

#include "crbase/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/callback.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/message_loop/pending_task.h"
#include "crbase/threading/thread_local.h"

namespace cr {

namespace {

ThreadLocalBoolean* GetWorkerPoolRunningOnThisThread() {
  static cr::NoDestructor<ThreadLocalBoolean> thread_local_boolean;
  return thread_local_boolean.get();
}

DWORD CALLBACK WorkItemCallback(void* param) {
  PendingTask* pending_task = static_cast<PendingTask*>(param);
  ///TRACE_TASK_EXECUTION("WorkerThread::ThreadMain::Run", *pending_task);

  GetWorkerPoolRunningOnThisThread()->Set(true);

  ///tracked_objects::TaskStopwatch stopwatch;
  ///stopwatch.Start();
  std::move(pending_task->task).Run();
  ///stopwatch.Stop();

  GetWorkerPoolRunningOnThisThread()->Set(false);

  ///tracked_objects::ThreadData::TallyRunOnWorkerThreadIfTracking(
  ///    pending_task->birth_tally, pending_task->time_posted, stopwatch);

  delete pending_task;
  return 0;
}

// Takes ownership of |pending_task|
bool PostTaskInternal(PendingTask* pending_task, bool task_is_slow) {
  // Use CHECK instead of DCHECK to crash earlier. See http://crbug.com/711167
  // for details.
  CR_CHECK(pending_task->task);

  ULONG flags = 0;
  if (task_is_slow)
    flags |= WT_EXECUTELONGFUNCTION;

  if (!QueueUserWorkItem(WorkItemCallback, pending_task, flags)) {
    CR_DPLOG(Error) << "QueueUserWorkItem failed";
    delete pending_task;
    return false;
  }

  return true;
}

}  // namespace

// static
bool WorkerPool::PostTask(const Location& from_here,
                          OnceClosure task,
                          bool task_is_slow) {
  PendingTask* pending_task = new PendingTask(from_here, std::move(task));
  return PostTaskInternal(pending_task, task_is_slow);
}

// static
bool WorkerPool::RunsTasksOnCurrentThread() {
  return GetWorkerPoolRunningOnThisThread()->Get();
}

}  // namespace cr
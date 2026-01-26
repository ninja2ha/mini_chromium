// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/threading/sequenced_task_runner_handle.h"

#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"
///#include "crbase/lazy_instance.h"

namespace cr {

namespace {

ThreadLocalPointer<SequencedTaskRunnerHandle>* GetSequencedTaskRunnerTls() {
  static cr::NoDestructor<ThreadLocalPointer<SequencedTaskRunnerHandle>> tls;
  return tls.get();
}

}  // namespace

// static
const RefPtr<SequencedTaskRunner>& SequencedTaskRunnerHandle::Get() {
  const SequencedTaskRunnerHandle* current =
      GetSequencedTaskRunnerTls()->Get();
  CR_CHECK(current)
      << "Error: This caller requires a sequenced context (i.e. the current "
         "task needs to run from a SequencedTaskRunner). If you're in a test "
         "refer to //docs/threading_and_tasks_testing.md.";
  return current->task_runner_;
}

// static
bool SequencedTaskRunnerHandle::IsSet() {
  return !!GetSequencedTaskRunnerTls()->Get();
}

SequencedTaskRunnerHandle::SequencedTaskRunnerHandle(
    RefPtr<SequencedTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)) {
  CR_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  CR_DCHECK(!SequencedTaskRunnerHandle::IsSet());
  GetSequencedTaskRunnerTls()->Set(this);
}

SequencedTaskRunnerHandle::~SequencedTaskRunnerHandle() {
  CR_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  CR_DCHECK(GetSequencedTaskRunnerTls()->Get() == this);
  GetSequencedTaskRunnerTls()->Set(nullptr);
}

}  // namespace cr
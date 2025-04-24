// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/task/sequenced_task_runner_handle.h"

#include <utility>

#include "crbase/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"

namespace cr {

namespace {

ThreadLocalPointer<SequencedTaskRunnerHandle>*
sequenced_task_runner_tls() {
  static cr::NoDestructor<
    ThreadLocalPointer<SequencedTaskRunnerHandle>> tls_task_runner;
  return tls_task_runner.get();
}

}  // namespace

// -- SequencedTaskRunnerHandle

// static
const RefPtr<SequencedTaskRunner>& SequencedTaskRunnerHandle::Get() {
  const SequencedTaskRunnerHandle* current =
      sequenced_task_runner_tls()->Get();
  CR_CHECK(current)
      << "Error: This caller requires a sequenced context (i.e. the current "
         "task needs to run from a SequencedTaskRunner). If you're in a test "
         "refer to //docs/threading_and_tasks_testing.md.";
  return current->task_runner_;
}

// static
bool SequencedTaskRunnerHandle::IsSet() {
  return !!sequenced_task_runner_tls()->Get();
}

SequencedTaskRunnerHandle::SequencedTaskRunnerHandle(
  RefPtr<SequencedTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)) {
  CR_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  CR_DCHECK(!SequencedTaskRunnerHandle::IsSet());
  sequenced_task_runner_tls()->Set(this);
}

SequencedTaskRunnerHandle::~SequencedTaskRunnerHandle() {
  CR_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  CR_DCHECK(sequenced_task_runner_tls()->Get() == this);
  sequenced_task_runner_tls()->Set(nullptr);
}
}  // namespace cr
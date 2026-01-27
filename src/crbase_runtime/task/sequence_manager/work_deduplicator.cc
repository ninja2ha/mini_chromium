// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/task/sequence_manager/work_deduplicator.h"

#include <utility>

#include "crbase/logging/logging.h"

namespace cr {
namespace sequence_manager {
namespace internal {

WorkDeduplicator::WorkDeduplicator(
    RefPtr<AssociatedThreadId> associated_thread)
    : associated_thread_(std::move(associated_thread)) {}

WorkDeduplicator::~WorkDeduplicator() = default;

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::BindToCurrentThread() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  int previous_flags = state_.fetch_or(kBoundFlag);
  CR_DCHECK((previous_flags & kBoundFlag) == 0) << "Can't bind twice!";
  return previous_flags & kPendingDoWorkFlag
             ? ShouldScheduleWork::kScheduleImmediate
             : ShouldScheduleWork::kNotNeeded;
}

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::OnWorkRequested() {
  // Set kPendingDoWorkFlag and return true if we were previously kIdle.
  return state_.fetch_or(kPendingDoWorkFlag) == State::kIdle
             ? ShouldScheduleWork::kScheduleImmediate
             : ShouldScheduleWork::kNotNeeded;
}

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::OnDelayedWorkRequested()
    const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  // This must be called on the associated thread or this read is racy.
  return state_.load() == State::kIdle ? ShouldScheduleWork::kScheduleImmediate
                                       : ShouldScheduleWork::kNotNeeded;
}

void WorkDeduplicator::OnWorkStarted() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  CR_DCHECK((state_.load() & kBoundFlag) == kBoundFlag);
  // Clear kPendingDoWorkFlag and mark us as in a DoWork.
  state_.store(State::kInDoWork);
}

void WorkDeduplicator::WillCheckForMoreWork() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  CR_DCHECK((state_.load() & kBoundFlag) == kBoundFlag);
  // Clear kPendingDoWorkFlag if it was set.
  state_.store(State::kInDoWork);
}

WorkDeduplicator::ShouldScheduleWork WorkDeduplicator::DidCheckForMoreWork(
    NextTask next_task) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
  CR_DCHECK((state_.load() & kBoundFlag) == kBoundFlag);
  if (next_task == NextTask::kIsImmediate) {
    state_.store(State::kDoWorkPending);
    return ShouldScheduleWork::kScheduleImmediate;
  }
  // If |next_task| is not immediate, there's still a possibility that
  // OnWorkRequested() was invoked racily from another thread just after this
  // thread determined that the next task wasn't immediate. In that case, that
  // other thread relies on us to return kScheduleImmediate.
  return (state_.fetch_and(~kInDoWorkFlag) & kPendingDoWorkFlag)
             ? ShouldScheduleWork::kScheduleImmediate
             : ShouldScheduleWork::kNotNeeded;
}

}  // namespace internal
}  // namespace sequence_manager
}  // namespace cr

// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/task/task_executor.h"

#include <type_traits>

#include "crbase/logging/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"
#include "crbase_runtime/task/task_traits.h"
#include "crbase_runtime/task/task_traits_extension.h"

namespace cr {

namespace {

// Maps TaskTraits extension IDs to registered TaskExecutors. Index |n|
// corresponds to id |n - 1|.
using TaskExecutorMap =
    std::array<TaskExecutor*, TaskTraitsExtensionStorage::kMaxExtensionId>;
TaskExecutorMap* GetTaskExecutorMap() {
  static_assert(std::is_trivially_destructible<TaskExecutorMap>::value,
                "TaskExecutorMap not trivially destructible");
  static TaskExecutorMap executors{};
  return &executors;
}

static_assert(
    TaskTraitsExtensionStorage::kInvalidExtensionId == 0,
    "TaskExecutorMap depends on 0 being an invalid TaskTraits extension ID");

}  // namespace

ThreadLocalPointer<TaskExecutor>* GetTLSForCurrentTaskExecutor() {
  static NoDestructor<ThreadLocalPointer<TaskExecutor>> instance;
  return instance.get();
}

void SetTaskExecutorForCurrentThread(TaskExecutor* task_executor) {
  CR_DCHECK(!task_executor || !GetTLSForCurrentTaskExecutor()->Get() ||
            GetTLSForCurrentTaskExecutor()->Get() == task_executor);
  GetTLSForCurrentTaskExecutor()->Set(task_executor);
}

TaskExecutor* GetTaskExecutorForCurrentThread() {
  return GetTLSForCurrentTaskExecutor()->Get();
}

void RegisterTaskExecutor(uint8_t extension_id, TaskExecutor* task_executor) {
  CR_DCHECK(extension_id != TaskTraitsExtensionStorage::kInvalidExtensionId);
  CR_DCHECK(extension_id <= TaskTraitsExtensionStorage::kMaxExtensionId);
  CR_DCHECK((*GetTaskExecutorMap())[extension_id - 1] == nullptr);

  (*GetTaskExecutorMap())[extension_id - 1] = task_executor;
}

void UnregisterTaskExecutorForTesting(uint8_t extension_id) {
  CR_DCHECK(extension_id != TaskTraitsExtensionStorage::kInvalidExtensionId);
  CR_DCHECK(extension_id <= TaskTraitsExtensionStorage::kMaxExtensionId);
  CR_DCHECK((*GetTaskExecutorMap())[extension_id - 1] != nullptr);

  (*GetTaskExecutorMap())[extension_id - 1] = nullptr;
}

TaskExecutor* GetRegisteredTaskExecutorForTraits(const TaskTraits& traits) {
  uint8_t extension_id = traits.extension_id();
  if (extension_id != TaskTraitsExtensionStorage::kInvalidExtensionId) {
    TaskExecutor* executor = (*GetTaskExecutorMap())[extension_id - 1];
    CR_DCHECK(executor)
        << "A TaskExecutor wasn't yet registered for this extension.\nHint: if "
           "this is in a unit test, you're likely missing a "
           "content::BrowserTaskEnvironment member in your fixture.";
    return executor;
  }

  return nullptr;
}

}  // namespace cr

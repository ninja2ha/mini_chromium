// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SINGLE_THREAD_TASK_EXECUTOR_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SINGLE_THREAD_TASK_EXECUTOR_H_

#include <memory>

#include "crbase/memory/ref_ptr.h"

#include "crbase_runtime/runtime_export.h"
#include "crbase_runtime/message_pump/message_pump_type.h"
#include "crbase_runtime/single_thread_task_runner.h"
#include "crbase_runtime/task/simple_task_executor.h"

namespace cr {

class MessagePump;

namespace sequence_manager {
class SequenceManager;
class TaskQueue;
}  // namespace sequence_manager

// A simple single thread TaskExecutor intended for non-test usage. Tests should
// generally use TaskEnvironment or BrowserTaskEnvironment instead.
// TODO(alexclarke): Inherit from TaskExecutor to support base::Here().
class CRBASE_RT_EXPORT SingleThreadTaskExecutor {
 public:
  SingleThreadTaskExecutor(const SingleThreadTaskExecutor&) = delete;
  SingleThreadTaskExecutor& operator=(const SingleThreadTaskExecutor&) = delete;

  // For MessagePumpType::CUSTOM use the constructor that takes a pump.
  explicit SingleThreadTaskExecutor(
      MessagePumpType type = MessagePumpType::DEFAULT);

  // Creates a SingleThreadTaskExecutor pumping from a custom |pump|.
  // The above constructor using MessagePumpType is generally preferred.
  explicit SingleThreadTaskExecutor(std::unique_ptr<MessagePump> pump);

  // Shuts down the SingleThreadTaskExecutor, after this no tasks can be
  // executed and the base::TaskExecutor APIs are non-functional but won't crash
  // if called.
  ~SingleThreadTaskExecutor();

  RefPtr<SingleThreadTaskRunner> task_runner() const;

  MessagePumpType type() const { return type_; }

  // Sets the number of application tasks executed every time the MessagePump
  // asks its delegate to DoWork(). Defaults to 1. Can be increased in some
  // scenarios where the native pump (i.e. not MessagePumpType::DEFAULT) has
  // high overhead and yielding to native isn't critical.
  void SetWorkBatchSize(size_t work_batch_size);

 private:
  explicit SingleThreadTaskExecutor(MessagePumpType type,
                                    std::unique_ptr<MessagePump> pump);

  std::unique_ptr<sequence_manager::SequenceManager> sequence_manager_;
  RefPtr<sequence_manager::TaskQueue> default_task_queue_;
  MessagePumpType type_;
  SimpleTaskExecutor simple_task_executor_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SINGLE_THREAD_TASK_EXECUTOR_H_

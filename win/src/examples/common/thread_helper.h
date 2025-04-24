// Copyright (c) 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_EXAMPLES_COMMON_MAIN_THREAD_HELPER_H_
#define MINI_CHROMIUM_SRC_EXAMPLES_COMMON_MAIN_THREAD_HELPER_H_

#include <memory>

#include "crbase/logging.h"
// already include run_loop.h
#include "crbase/message_loop/message_loop.h" 
#include "crbase/threading/platform_thread.h"
#include "crbase/threading/thread.h"

namespace example {

class MainThreadHelper final {
 public:
  MainThreadHelper(cr::MessageLoop::Type type, bool nestable)
    : message_loop_(new cr::MessageLoop(type)),
      run_loop_(new cr::RunLoop),
      owner_tid_(cr::PlatformThread::CurrentId()) {
    message_loop_->SetNestableTasksAllowed(nestable);
    instant_ = this;
  }

  ~MainThreadHelper() { instant_ = nullptr; };

  void Run() {
    run_loop_->Run();
  }

  void AsyncQuit() {
    run_loop_->Quit();
  }

  const cr::RefPtr<cr::SingleThreadTaskRunner>& task_runner() {
    return message_loop_->task_runner();
  }

  bool IsOnCurrentlyThread() {
    return owner_tid_ == cr::PlatformThread::CurrentId();
  }

  static MainThreadHelper* Get() { return instant_; }

 private:
  cr::PlatformThreadId owner_tid_;

  std::unique_ptr<cr::MessageLoop> message_loop_;
  std::unique_ptr<cr::RunLoop> run_loop_;

  static MainThreadHelper* instant_;
};

MainThreadHelper* MainThreadHelper::instant_ = nullptr;

// - IOThreadHelper

class IOThreadHelper final {
 public:
  explicit IOThreadHelper(const std::string& name) : name_(name) {}
  ~IOThreadHelper() = default;

  bool Start(bool joinable) {
    if (!thread_)
      thread_ = std::make_unique<cr::Thread>(name_);

    if (thread_->IsRunning())
      return true;

    cr::Thread::Options options;
    options.message_loop_type = cr::MessageLoop::TYPE_IO;
    options.joinable = joinable;
    return thread_->StartWithOptions(options);
  }

  cr::RefPtr<cr::SingleThreadTaskRunner> task_runner() {
    return thread_ ? thread_->task_runner() : nullptr;
  }

  bool IsOnCurrentlyThread() {
    CR_DCHECK(thread_);
    return thread_->GetThreadId() == cr::PlatformThread::CurrentId();
  }

  static std::unique_ptr<IOThreadHelper> Create(const std::string& name) {
    return std::make_unique<IOThreadHelper>(name);
  }

 private:
  std::string name_;
  std::unique_ptr<cr::Thread> thread_;
};

}  // namespace example

#endif  // MINI_CHROMIUM_SRC_EXAMPLES_COMMON_MAIN_THREAD_HELPER_H_
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/task/current_thread.h"

#include "crbase/functional/bind.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"
#include "crbase_runtime/message_pump/message_pump_for_io.h"
#include "crbase_runtime/message_pump/message_pump_for_ui.h"
#include "crbase_runtime/message_pump/message_pump_type.h"
#include "crbase_runtime/task/sequence_manager/sequence_manager_impl.h"
#include "crbase_runtime/threading/thread_task_runner_handle.h"

namespace cr {

//------------------------------------------------------------------------------
// CurrentThread

// static
sequence_manager::internal::SequenceManagerImpl*
CurrentThread::GetCurrentSequenceManagerImpl() {
  return sequence_manager::internal::SequenceManagerImpl::GetCurrent();
}

// static
CurrentThread CurrentThread::Get() {
  return CurrentThread(GetCurrentSequenceManagerImpl());
}

// static
CurrentThread CurrentThread::GetNull() {
  return CurrentThread(nullptr);
}

// static
bool CurrentThread::IsSet() {
  return !!GetCurrentSequenceManagerImpl();
}

void CurrentThread::AddDestructionObserver(
    DestructionObserver* destruction_observer) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->AddDestructionObserver(destruction_observer);
}

void CurrentThread::RemoveDestructionObserver(
    DestructionObserver* destruction_observer) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->RemoveDestructionObserver(destruction_observer);
}

void CurrentThread::SetTaskRunner(
    RefPtr<SingleThreadTaskRunner> task_runner) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->SetTaskRunner(std::move(task_runner));
}

bool CurrentThread::IsBoundToCurrentThread() const {
  return current_ == GetCurrentSequenceManagerImpl();
}

bool CurrentThread::IsIdleForTesting() {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  return current_->IsIdleForTesting();
}

void CurrentThread::AddTaskObserver(TaskObserver* task_observer) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->AddTaskObserver(task_observer);
}

void CurrentThread::RemoveTaskObserver(TaskObserver* task_observer) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->RemoveTaskObserver(task_observer);
}

void CurrentThread::AddTaskTimeObserver(
    sequence_manager::TaskTimeObserver* task_observer) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->AddTaskTimeObserver(task_observer);
}

void CurrentThread::RemoveTaskTimeObserver(
    sequence_manager::TaskTimeObserver* task_observer) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->RemoveTaskTimeObserver(task_observer);
}

void CurrentThread::SetAddQueueTimeToTasks(bool enable) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  current_->SetAddQueueTimeToTasks(enable);
}

CurrentThread::ScopedAllowApplicationTasksInNativeNestedLoop::
    ScopedAllowApplicationTasksInNativeNestedLoop()
    : sequence_manager_(GetCurrentSequenceManagerImpl()),
      previous_state_(sequence_manager_->IsTaskExecutionAllowed()) {
  ///TRACE_EVENT_BEGIN0("base", "ScopedNestableTaskAllower");
  sequence_manager_->SetTaskExecutionAllowed(true);
}

CurrentThread::ScopedAllowApplicationTasksInNativeNestedLoop::
    ~ScopedAllowApplicationTasksInNativeNestedLoop() {
  sequence_manager_->SetTaskExecutionAllowed(previous_state_);
  ///TRACE_EVENT_END0("base", "ScopedNestableTaskAllower");
}

bool CurrentThread::NestableTasksAllowed() const {
  return current_->IsTaskExecutionAllowed();
}

bool CurrentThread::operator==(const CurrentThread& other) const {
  return current_ == other.current_;
}

//------------------------------------------------------------------------------
// CurrentUIThread

// static
CurrentUIThread CurrentUIThread::Get() {
  auto* sequence_manager = GetCurrentSequenceManagerImpl();
  CR_DCHECK(sequence_manager);
  CR_DCHECK(sequence_manager->IsType(MessagePumpType::UI));
  return CurrentUIThread(sequence_manager);
}

// static
bool CurrentUIThread::IsSet() {
  sequence_manager::internal::SequenceManagerImpl* sequence_manager =
      GetCurrentSequenceManagerImpl();
  return sequence_manager &&
         sequence_manager->IsType(MessagePumpType::UI);
}

MessagePumpForUI* CurrentUIThread::GetMessagePumpForUI() const {
  return static_cast<MessagePumpForUI*>(current_->GetMessagePump());
}

#if !defined(MINI_CHROMIUM_OS_WIN)
bool CurrentUIThread::WatchFileDescriptor(
    int fd,
    bool persistent,
    MessagePumpForUI::Mode mode,
    MessagePumpForUI::FdWatchController* controller,
    MessagePumpForUI::FdWatcher* delegate) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  return GetMessagePumpForUI()->WatchFileDescriptor(fd, persistent, mode,
                                                    controller, delegate);
}
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
void CurrentUIThread::AddMessagePumpObserver(
    MessagePumpForUI::Observer* observer) {
  GetMessagePumpForUI()->AddObserver(observer);
}

void CurrentUIThread::RemoveMessagePumpObserver(
    MessagePumpForUI::Observer* observer) {
  GetMessagePumpForUI()->RemoveObserver(observer);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)


//------------------------------------------------------------------------------
// CurrentIOThread

// static
CurrentIOThread CurrentIOThread::Get() {
  auto* sequence_manager = GetCurrentSequenceManagerImpl();
  CR_DCHECK(sequence_manager);
  CR_DCHECK(sequence_manager->IsType(MessagePumpType::IO));
  return CurrentIOThread(sequence_manager);
}

// static
bool CurrentIOThread::IsSet() {
  auto* sequence_manager = GetCurrentSequenceManagerImpl();
  return sequence_manager && sequence_manager->IsType(MessagePumpType::IO);
}

MessagePumpForIO* CurrentIOThread::GetMessagePumpForIO() const {
  return static_cast<MessagePumpForIO*>(current_->GetMessagePump());
}

#if defined(MINI_CHROMIUM_OS_WIN)
HRESULT CurrentIOThread::RegisterIOHandler(
    HANDLE file,
    MessagePumpForIO::IOHandler* handler) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  return GetMessagePumpForIO()->RegisterIOHandler(file, handler);
}

bool CurrentIOThread::RegisterJobObject(HANDLE job,
                                        MessagePumpForIO::IOHandler* handler) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  return GetMessagePumpForIO()->RegisterJobObject(job, handler);
}

#elif defined(MINI_CHROMIUM_OS_POSIX)
bool CurrentIOThread::WatchFileDescriptor(
    int fd,
    bool persistent,
    MessagePumpForIO::Mode mode,
    MessagePumpForIO::FdWatchController* controller,
    MessagePumpForIO::FdWatcher* delegate) {
  CR_DCHECK(current_->IsBoundToCurrentThread());
  return GetMessagePumpForIO()->WatchFileDescriptor(fd, persistent, mode,
                                                    controller, delegate);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)


}  // namespace cr

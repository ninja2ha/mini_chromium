// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 71.0.3578.141

#include "crbase_runtime/message_loop/message_loop_current.h"

#include "crbase/functional/bind.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"
#include "crbase_runtime/message_loop/message_loop.h"
#include "crbase_runtime/message_pump/message_pump_for_io.h"
#include "crbase_runtime/message_pump/message_pump_for_ui.h"

namespace cr {

namespace {

cr::ThreadLocalPointer<MessageLoop>* GetTLSMessageLoop() {
  static NoDestructor<ThreadLocalPointer<MessageLoop>> lazy_tls_ptr;
  return lazy_tls_ptr.get();
}

}  // namespace

//------------------------------------------------------------------------------
// MessageLoopCurrent

// static
MessageLoopCurrent MessageLoopCurrent::Get() {
  return MessageLoopCurrent(GetTLSMessageLoop()->Get());
}

// static
bool MessageLoopCurrent::IsSet() {
  return !!GetTLSMessageLoop()->Get();
}

void MessageLoopCurrent::AddDestructionObserver(
    DestructionObserver* destruction_observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->destruction_observers_.AddObserver(destruction_observer);
}

void MessageLoopCurrent::RemoveDestructionObserver(
    DestructionObserver* destruction_observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->destruction_observers_.RemoveObserver(destruction_observer);
}

bool MessageLoopCurrent::IsIdleForTesting() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return current_->IsIdleForTesting();
}

void MessageLoopCurrent::SetAddQueueTimeToTasks(bool enable) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->SetAddQueueTimeToTasks(enable);
}

// static
void MessageLoopCurrent::BindToCurrentThreadInternal(MessageLoop* current) {
  CR_DCHECK(!GetTLSMessageLoop()->Get())
      << "Can't register a second MessageLoop on the same thread.";
  GetTLSMessageLoop()->Set(current);
}

// static
void MessageLoopCurrent::UnbindFromCurrentThreadInternal(MessageLoop* current) {
  CR_DCHECK(current == GetTLSMessageLoop()->Get());
  GetTLSMessageLoop()->Set(nullptr);
}

bool MessageLoopCurrent::IsBoundToCurrentThreadInternal(
    MessageLoop* message_loop) {
  return GetTLSMessageLoop()->Get() == message_loop;
}

//------------------------------------------------------------------------------
// MessageLoopCurrentForUI

// static
MessageLoopCurrentForUI MessageLoopCurrentForUI::Get() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  CR_DCHECK(loop);
  CR_DCHECK(loop->IsType(MessageLoop::TYPE_UI));
  auto* loop_for_ui = static_cast<MessageLoopForUI*>(loop);
  return MessageLoopCurrentForUI(
      loop_for_ui, static_cast<MessagePumpForUI*>(loop_for_ui->pump_.get()));
}

// static
bool MessageLoopCurrentForUI::IsSet() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  return loop &&
         loop->IsType(MessageLoop::TYPE_UI);
}

#if !defined(MINI_CHROMIUM_OS_WIN)
bool MessageLoopCurrentForUI::WatchFileDescriptor(
    int fd,
    bool persistent,
    MessagePumpForUI::Mode mode,
    MessagePumpForUI::FdWatchController* controller,
    MessagePumpForUI::FdWatcher* delegate) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->WatchFileDescriptor(fd, persistent, mode, controller, delegate);
}
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
void MessageLoopCurrentForUI::AddMessagePumpObserver(
    MessagePumpForUI::Observer* observer) {
  pump_->AddObserver(observer);
}

void MessageLoopCurrentForUI::RemoveMessagePumpObserver(
    MessagePumpForUI::Observer* observer) {
  pump_->RemoveObserver(observer);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

//------------------------------------------------------------------------------
// MessageLoopCurrentForIO

// static
MessageLoopCurrentForIO MessageLoopCurrentForIO::Get() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  CR_DCHECK(loop);
  CR_DCHECK(MessageLoop::TYPE_IO == loop->type());
  auto* loop_for_io = static_cast<MessageLoopForIO*>(loop);
  return MessageLoopCurrentForIO(
      loop_for_io, static_cast<MessagePumpForIO*>(loop_for_io->pump_.get()));
}

// static
bool MessageLoopCurrentForIO::IsSet() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  return loop && loop->IsType(MessageLoop::TYPE_IO);
}

#if defined(MINI_CHROMIUM_OS_WIN)
HRESULT MessageLoopCurrentForIO::RegisterIOHandler(
    HANDLE file,
    MessagePumpForIO::IOHandler* handler) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->RegisterIOHandler(file, handler);
}

bool MessageLoopCurrentForIO::RegisterJobObject(
    HANDLE job,
    MessagePumpForIO::IOHandler* handler) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->RegisterJobObject(job, handler);
}

///bool MessageLoopCurrentForIO::WaitForIOCompletion(
///    DWORD timeout,
///    MessagePumpForIO::IOHandler* filter) {
///  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
///  return pump_->WaitForIOCompletion(timeout, filter);
///}
#elif defined(MINI_CHROMIUM_OS_POSIX)
bool MessageLoopCurrentForIO::WatchFileDescriptor(
    int fd,
    bool persistent,
    MessagePumpForIO::Mode mode,
    MessagePumpForIO::FdWatchController* controller,
    MessagePumpForIO::FdWatcher* delegate) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->WatchFileDescriptor(fd, persistent, mode, controller, delegate);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

}  // namespace cr
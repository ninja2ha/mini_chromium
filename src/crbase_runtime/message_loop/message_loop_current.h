// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 71.0.3578.141

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_

#include "crbase/base_export.h"
#include "crbase/logging/logging.h"
#include "crbase/memory/ref_ptr.h"
#include "crbase_runtime/message_pump/message_pump_for_io.h"
#include "crbase_runtime/message_pump/message_pump_for_ui.h"
#include "crbase_runtime/pending_task.h"
#include "crbase_runtime/single_thread_task_runner.h"
#include "crbuild/build_config.h"

namespace cr {

class MessageLoop;

// MessageLoopCurrent is a proxy to the public interface of the MessageLoop
// bound to the thread it's obtained on.
//
// MessageLoopCurrent(ForUI|ForIO) is available statically through
// MessageLoopCurrent(ForUI|ForIO)::Get() on threads that have a matching
// MessageLoop instance. APIs intended for all consumers on the thread should be
// on MessageLoopCurrent(ForUI|ForIO), while APIs intended for the owner of the
// instance should be on MessageLoop(ForUI|ForIO).
//
// Why: Historically MessageLoop::current() gave access to the full MessageLoop
// API, preventing both addition of powerful owner-only APIs as well as making
// it harder to remove callers of deprecated APIs (that need to stick around for
// a few owner-only use cases and re-accrue callers after cleanup per remaining
// publicly available).
//
// As such, many methods below are flagged as deprecated and should be removed
// (or moved back to MessageLoop) once all static callers have been migrated.
class CRBASE_EXPORT MessageLoopCurrent {
 public:
  // MessageLoopCurrent is effectively just a disguised pointer and is fine to
  // copy/move around.
  MessageLoopCurrent(const MessageLoopCurrent& other) = default;
  MessageLoopCurrent(MessageLoopCurrent&& other) = default;

  // Returns a proxy object to interact with the MessageLoop running the
  // current thread. It must only be used on the thread it was obtained.
  static MessageLoopCurrent Get();

  // Returns true if the current thread is running a MessageLoop. Prefer this to
  // verifying the boolean value of Get() (so that Get() can ultimately DCHECK
  // it's only invoked when IsSet()).
  static bool IsSet();

  // Allow MessageLoopCurrent to be used like a pointer to support the many
  // callsites that used MessageLoop::current() that way when it was a
  // MessageLoop*.
  MessageLoopCurrent* operator->() { return this; }
  explicit operator bool() const { return !!current_; }

  // TODO(gab): Migrate the types of variables that store MessageLoop::current()
  // and remove this implicit cast back to MessageLoop*.
  operator MessageLoop*() const { return current_; }

  // A DestructionObserver is notified when the current MessageLoop is being
  // destroyed.  These observers are notified prior to MessageLoop::current()
  // being changed to return NULL.  This gives interested parties the chance to
  // do final cleanup that depends on the MessageLoop.
  //
  // NOTE: Any tasks posted to the MessageLoop during this notification will
  // not be run.  Instead, they will be deleted.
  //
  // Deprecation note: Prefer SequenceLocalStorageSlot<std::unique_ptr<Foo>> to
  // DestructionObserver to bind an object's lifetime to the current
  // thread/sequence.
  class CRBASE_EXPORT DestructionObserver {
   public:
    virtual void WillDestroyCurrentMessageLoop() = 0;

   protected:
    virtual ~DestructionObserver() = default;
  };

  // Add a DestructionObserver, which will start receiving notifications
  // immediately.
  void AddDestructionObserver(DestructionObserver* destruction_observer);

  // Remove a DestructionObserver.  It is safe to call this method while a
  // DestructionObserver is receiving a notification callback.
  void RemoveDestructionObserver(DestructionObserver* destruction_observer);

  // A TaskObserver is an object that receives task notifications from the
  // MessageLoop.
  //
  // NOTE: A TaskObserver implementation should be extremely fast!
  class CRBASE_EXPORT TaskObserver {
   public:
    // This method is called before processing a task.
    virtual void WillProcessTask(const PendingTask& pending_task) = 0;

    // This method is called after processing a task.
    virtual void DidProcessTask(const PendingTask& pending_task) = 0;

   protected:
    virtual ~TaskObserver() = default;
  };

  // When this functionality is enabled, the queue time will be recorded for
  // posted tasks.
  void SetAddQueueTimeToTasks(bool enable);

  // Returns true if the message loop is idle (ignoring delayed tasks). This is
  // the same condition which triggers DoWork() to return false: i.e.
  // out of tasks which can be processed at the current run-level -- there might
  // be deferred non-nestable tasks remaining if currently in a nested run
  // level.
  bool IsIdleForTesting();

  // Binds |current| to the current thread. It will from then on be the
  // MessageLoop driven by MessageLoopCurrent on this thread. This is only meant
  // to be invoked by the MessageLoop itself.
  static void BindToCurrentThreadInternal(MessageLoop* current);

  // Unbinds |current| from the current thread. Must be invoked on the same
  // thread that invoked |BindToCurrentThreadInternal(current)|. This is only
  // meant to be invoked by the MessageLoop itself.
  static void UnbindFromCurrentThreadInternal(MessageLoop* current);

  // Returns true if |message_loop| is bound to MessageLoopCurrent on the
  // current thread. This is only meant to be invoked by the MessageLoop itself.
  static bool IsBoundToCurrentThreadInternal(MessageLoop* message_loop);

 protected:
  explicit MessageLoopCurrent(MessageLoop* current) : current_(current) {}

  MessageLoop* const current_;
};

// ForUI extension of MessageLoopCurrent.
class CRBASE_EXPORT MessageLoopCurrentForUI : public MessageLoopCurrent {
 public:
  // Returns an interface for the MessageLoopForUI of the current thread.
  // Asserts that IsSet().
  static MessageLoopCurrentForUI Get();

  // Returns true if the current thread is running a MessageLoopForUI.
  static bool IsSet();

  MessageLoopCurrentForUI* operator->() { return this; }

#if !defined(MINI_CHROMIUM_OS_WIN)
  // Please see MessagePumpLibevent for definition.
  static_assert(std::is_same<MessagePumpForUI, MessagePumpLibevent>::value,
                "MessageLoopCurrentForUI::WatchFileDescriptor is not supported "
                "when MessagePumpForUI is not a MessagePumpLibevent.");
  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           MessagePumpForUI::Mode mode,
                           MessagePumpForUI::FdWatchController* controller,
                           MessagePumpForUI::FdWatcher* delegate);
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
  void AddMessagePumpObserver(MessagePumpForUI::Observer* observer);
  void RemoveMessagePumpObserver(MessagePumpForUI::Observer* observer);
#endif

 private:
  MessageLoopCurrentForUI(MessageLoop* current, MessagePumpForUI* pump)
      : MessageLoopCurrent(current), pump_(pump) {
    CR_DCHECK(pump_);
  }

  MessagePumpForUI* const pump_;
};

// ForIO extension of MessageLoopCurrent.
class CRBASE_EXPORT MessageLoopCurrentForIO : public MessageLoopCurrent {
 public:
  // Returns an interface for the MessageLoopForIO of the current thread.
  // Asserts that IsSet().
  static MessageLoopCurrentForIO Get();

  // Returns true if the current thread is running a MessageLoopForIO.
  static bool IsSet();

  MessageLoopCurrentForIO* operator->() { return this; }


#if defined(MINI_CHROMIUM_OS_WIN)
  // Please see MessagePumpWin for definitions of these methods.
  HRESULT RegisterIOHandler(HANDLE file, MessagePumpForIO::IOHandler* handler);
  bool RegisterJobObject(HANDLE job, MessagePumpForIO::IOHandler* handler);
  ///bool WaitForIOCompletion(DWORD timeout, MessagePumpForIO::IOHandler* filter);
#elif defined(MINI_CHROMIUM_OS_POSIX)
  // Please see WatchableIOMessagePumpPosix for definition.
  // Prefer base::FileDescriptorWatcher for non-critical IO.
  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           MessagePumpForIO::Mode mode,
                           MessagePumpForIO::FdWatchController* controller,
                           MessagePumpForIO::FdWatcher* delegate);
#endif  // defined(MINI_CHROMIUM_OS_WIN)

 private:
  MessageLoopCurrentForIO(MessageLoop* current, MessagePumpForIO* pump)
      : MessageLoopCurrent(current), pump_(pump) {
    CR_DCHECK(pump_);
  }

  MessagePumpForIO* const pump_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_MESSAGE_LOOP_MESSAGE_LOOP_CURRENT_H_
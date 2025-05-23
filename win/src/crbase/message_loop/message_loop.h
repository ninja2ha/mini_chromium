// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_MESSAGE_LOOP_H_
#define MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_MESSAGE_LOOP_H_

#include <memory>
#include <queue>
#include <string>

#include "crbase/base_export.h"
#include "crbase/functional/callback_forward.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/message_loop/incoming_task_queue.h"
#include "crbase/message_loop/message_loop_task_runner.h"
#include "crbase/message_loop/message_pump.h"
#include "crbase/message_loop/timer_slack.h"
#include "crbase/message_loop/pending_task.h"
#include "crbase/message_loop/run_loop.h"
#include "crbase/observer_list.h"
#include "crbase/synchronization/lock.h"
#include "crbase/time/time.h"
#include "crbase/build_platform.h"

// TODO(sky): these includes should not be necessary. Nuke them.
#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/message_loop/message_pump_win.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include "crbase/message_loop/message_pump_libevent.h"
#endif

namespace cr {

class ThreadTaskRunnerHandle;
class WaitableEvent;

// A MessageLoop is used to process events for a particular thread.  There is
// at most one MessageLoop instance per thread.
//
// Events include at a minimum Task instances submitted to the MessageLoop's
// TaskRunner. Depending on the type of message pump used by the MessageLoop
// other events such as UI messages may be processed.  On Windows APC calls (as
// time permits) and signals sent to a registered set of HANDLEs may also be
// processed.
//
// NOTE: Unless otherwise specified, a MessageLoop's methods may only be called
// on the thread where the MessageLoop's Run method executes.
//
// NOTE: MessageLoop has task reentrancy protection.  This means that if a
// task is being processed, a second task cannot start until the first task is
// finished.  Reentrancy can happen when processing a task, and an inner
// message pump is created.  That inner pump then processes native messages
// which could implicitly start an inner task.  Inner message pumps are created
// with dialogs (DialogBox), common dialogs (GetOpenFileName), OLE functions
// (DoDragDrop), printer functions (StartDoc) and *many* others.
//
// Sample workaround when inner task processing is needed:
//   HRESULT hr;
//   {
//     MessageLoop::ScopedNestableTaskAllower allow(MessageLoop::current());
//     hr = DoDragDrop(...); // Implicitly runs a modal message loop.
//   }
//   // Process |hr| (the result returned by DoDragDrop()).
//
// Please be SURE your task is reentrant (nestable) and all global variables
// are stable and accessible before calling SetNestableTasksAllowed(true).
//
class CRBASE_EXPORT MessageLoop : public MessagePump::Delegate,
                                  public RunLoop::Delegate {
 public:
  // A MessageLoop has a particular type, which indicates the set of
  // asynchronous events it may process in addition to tasks and timers.
  //
  // TYPE_DEFAULT
  //   This type of ML only supports tasks and timers.
  //
  // TYPE_UI
  //   This type of ML also supports native UI events (e.g., Windows messages).
  //   See also MessageLoopForUI.
  //
  // TYPE_IO
  //   This type of ML also supports asynchronous IO.  See also
  //   MessageLoopForIO.
  //
  // TYPE_JAVA
  //   This type of ML is backed by a Java message handler which is responsible
  //   for running the tasks added to the ML. This is only for use on Android.
  //   TYPE_JAVA behaves in essence like TYPE_UI, except during construction
  //   where it does not use the main thread specific pump factory.
  //
  // TYPE_CUSTOM
  //   MessagePump was supplied to constructor.
  //
  enum Type {
    TYPE_DEFAULT,
    TYPE_UI,
    TYPE_CUSTOM,
    TYPE_IO,
  };

  MessageLoop(const MessageLoop&) = delete;
  MessageLoop& operator=(const MessageLoop&) = delete;

  // Normally, it is not necessary to instantiate a MessageLoop.  Instead, it
  // is typical to make use of the current thread's MessageLoop instance.
  explicit MessageLoop(Type type = TYPE_DEFAULT);
  // Creates a TYPE_CUSTOM MessageLoop with the supplied MessagePump, which must
  // be non-NULL.
  explicit MessageLoop(std::unique_ptr<MessagePump> pump);

  ~MessageLoop() override;

  // Returns the MessageLoop object for the current thread, or null if none.
  static MessageLoop* current();

  typedef std::unique_ptr<MessagePump>(MessagePumpFactory)();
  // Uses the given base::MessagePumpForUIFactory to override the default
  // MessagePump implementation for 'TYPE_UI'. Returns true if the factory
  // was successfully registered.
  static bool InitMessagePumpForUIFactory(MessagePumpFactory* factory);

  // Creates the default MessagePump based on |type|. Caller owns return
  // value.
  static std::unique_ptr<MessagePump> CreateMessagePumpForType(Type type);

  // A DestructionObserver is notified when the current MessageLoop is being
  // destroyed.  These observers are notified prior to MessageLoop::current()
  // being changed to return NULL.  This gives interested parties the chance to
  // do final cleanup that depends on the MessageLoop.
  //
  // NOTE: Any tasks posted to the MessageLoop during this notification will
  // not be run.  Instead, they will be deleted.
  //
  class CRBASE_EXPORT DestructionObserver {
   public:
    virtual void WillDestroyCurrentMessageLoop() = 0;

   protected:
    virtual ~DestructionObserver();
  };

  // Add a DestructionObserver, which will start receiving notifications
  // immediately.
  void AddDestructionObserver(DestructionObserver* destruction_observer);

  // Remove a DestructionObserver.  It is safe to call this method while a
  // DestructionObserver is receiving a notification callback.
  void RemoveDestructionObserver(DestructionObserver* destruction_observer);

  // Deprecated: use RunLoop instead.
  //
  // Signals the Run method to return when it becomes idle. It will continue to
  // process pending messages and future messages as long as they are enqueued.
  // Warning: if the MessageLoop remains busy, it may never quit. Only use this
  // Quit method when looping procedures (such as web pages) have been shut
  // down.
  //
  // This method may only be called on the same thread that called Run, and Run
  // must still be on the call stack.
  //
  // Use QuitClosure variants if you need to Quit another thread's MessageLoop,
  // but note that doing so is fairly dangerous if the target thread makes
  // nested calls to MessageLoop::Run.  The problem being that you won't know
  // which nested run loop you are quitting, so be careful!
  void QuitWhenIdle();

  // Deprecated: use RunLoop instead.
  //
  // This method is a variant of Quit, that does not wait for pending messages
  // to be processed before returning from Run.
  void QuitNow();

  // Set the timer slack for this message loop.
  void SetTimerSlack(TimerSlack timer_slack) {
    pump_->SetTimerSlack(timer_slack);
  }

  // Returns true if this loop is |type|. This allows subclasses (especially
  // those in tests) to specialize how they are identified.
  virtual bool IsType(Type type) const;

  // Returns the type passed to the constructor.
  Type type() const { return type_; }

  // Returns the name of the thread this message loop is bound to. This function
  // is only valid when this message loop is running, BindToCurrentThread has
  // already been called and has an "happens-before" relationship with this call
  // (this relationship is obtained implicitly by the MessageLoop's task posting
  // system unless calling this very early).
  std::string GetThreadName() const;

  // Gets the TaskRunner associated with this message loop.
  const RefPtr<SingleThreadTaskRunner>& task_runner() {
    return task_runner_;
  }

  // Sets a new TaskRunner for this message loop. The message loop must already
  // have been bound to a thread prior to this call, and the task runner must
  // belong to that thread. Note that changing the task runner will also affect
  // the ThreadTaskRunnerHandle for the target thread. Must be called on the
  // thread to which the message loop is bound.
  void SetTaskRunner(RefPtr<SingleThreadTaskRunner> task_runner);

  // Enables or disables the recursive task processing. This happens in the case
  // of recursive message loops. Some unwanted message loops may occur when
  // using common controls or printer functions. By default, recursive task
  // processing is disabled.
  //
  // Please use |ScopedNestableTaskAllower| instead of calling these methods
  // directly.  In general, nestable message loops are to be avoided.  They are
  // dangerous and difficult to get right, so please use with extreme caution.
  //
  // The specific case where tasks get queued is:
  // - The thread is running a message loop.
  // - It receives a task #1 and executes it.
  // - The task #1 implicitly starts a message loop, like a MessageBox in the
  //   unit test. This can also be StartDoc or GetSaveFileName.
  // - The thread receives a task #2 before or while in this second message
  //   loop.
  // - With NestableTasksAllowed set to true, the task #2 will run right away.
  //   Otherwise, it will get executed right after task #1 completes at "thread
  //   message loop level".
  void SetNestableTasksAllowed(bool allowed);
  bool NestableTasksAllowed() const;

  // Enables nestable tasks on |loop| while in scope.
  class ScopedNestableTaskAllower {
   public:
    explicit ScopedNestableTaskAllower(MessageLoop* loop)
        : loop_(loop),
          old_state_(loop_->NestableTasksAllowed()) {
      loop_->SetNestableTasksAllowed(true);
    }
    ~ScopedNestableTaskAllower() {
      loop_->SetNestableTasksAllowed(old_state_);
    }

   private:
    MessageLoop* loop_;
    bool old_state_;
  };

  // A TaskObserver is an object that receives task notifications from the
  // MessageLoop.
  //
  // NOTE: A TaskObserver implementation should be extremely fast!
  class CRBASE_EXPORT TaskObserver {
   public:
    TaskObserver();

    // This method is called before processing a task.
    virtual void WillProcessTask(const PendingTask& pending_task) = 0;

    // This method is called after processing a task.
    virtual void DidProcessTask(const PendingTask& pending_task) = 0;

   protected:
    virtual ~TaskObserver();
  };

  // These functions can only be called on the same thread that |this| is
  // running on.
  void AddTaskObserver(TaskObserver* task_observer);
  void RemoveTaskObserver(TaskObserver* task_observer);

  // Returns true if the message loop has high resolution timers enabled.
  // Provided for testing.
  bool HasHighResolutionTasks();

  // Runs the specified PendingTask.
  void RunTask(PendingTask* pending_task);

  // Disallow task observers. After this is called, calling
  // Add/RemoveTaskObserver() on this MessageLoop will crash.
  void DisallowTaskObservers() { allow_task_observers_ = false; }

  //----------------------------------------------------------------------------
 protected:
  std::unique_ptr<MessagePump> pump_;

  using MessagePumpFactoryCallback =
      OnceCallback<std::unique_ptr<MessagePump>()>;

  // Common protected constructor. Other constructors delegate the
  // initialization to this constructor.
  // A subclass can invoke this constructor to create a message_loop of a
  // specific type with a custom loop. The implementation does not call
  // BindToCurrentThread. If this constructor is invoked directly by a subclass,
  // then the subclass must subsequently bind the message loop.
  MessageLoop(Type type, MessagePumpFactoryCallback pump_factory);

  // Configure various members and bind this message loop to the current thread.
  void BindToCurrentThread();

 private:
  friend class internal::IncomingTaskQueue;
  friend class Thread;
  friend struct PendingTask;

  // Creates a MessageLoop without binding to a thread.
  // If |type| is TYPE_CUSTOM non-null |pump_factory| must be also given
  // to create a message pump for this message loop.  Otherwise a default
  // message pump for the |type| is created.
  //
  // It is valid to call this to create a new message loop on one thread,
  // and then pass it to the thread where the message loop actually runs.
  // The message loop's BindToCurrentThread() method must be called on the
  // thread the message loop runs on, before calling Run().
  // Before BindToCurrentThread() is called, only Post*Task() functions can
  // be called on the message loop.
  static std::unique_ptr<MessageLoop> CreateUnbound(
      Type type,
      MessagePumpFactoryCallback pump_factory);

  // Sets the ThreadTaskRunnerHandle for the current thread to point to the
  // task runner for this message loop.
  void SetThreadTaskRunnerHandle();

  // RunLoop::Delegate:
  void Run() override;
  void Quit() override;

  // Called to process any delayed non-nestable tasks.
  bool ProcessNextDelayedNonNestableTask();

  // Calls RunTask or queues the pending_task on the deferred task list if it
  // cannot be run right now.  Returns true if the task was run.
  bool DeferOrRunPendingTask(PendingTask pending_task);

  // Adds the pending task to delayed_work_queue_.
  void AddToDelayedWorkQueue(PendingTask pending_task);

  // Sweeps any cancelled tasks from the front of the delayed work queue and
  // returns true if there is remaining work.
  bool SweepDelayedWorkQueueAndReturnTrueIfStillHasWork();

  // Delete tasks that haven't run yet without running them.  Used in the
  // destructor to make sure all the task's destructors get called.  Returns
  // true if some work was done.
  bool DeletePendingTasks();

  // Loads tasks from the incoming queue to |work_queue_| if the latter is
  // empty.
  void ReloadWorkQueue();

  // Wakes up the message pump. Can be called on any thread. The caller is
  // responsible for synchronizing ScheduleWork() calls.
  void ScheduleWork();

  // MessagePump::Delegate methods:
  bool DoWork() override;
  bool DoDelayedWork(TimeTicks* next_delayed_work_time) override;
  bool DoIdleWork() override;

  const Type type_;

  // A list of tasks that need to be processed by this instance.  Note that
  // this queue is only accessed (push/pop) by our current thread.
  TaskQueue work_queue_;

#if defined(MINI_CHROMIUM_OS_WIN)
  // Helper to decrement the high resolution task count if |pending_task| is a
  // high resolution task.
  void DecrementHighResTaskCountIfNeeded(const PendingTask& pending_Task);

  // How many high resolution tasks are in the pending task queue. This value
  // increases by N every time we call ReloadWorkQueue() and decreases by 1
  // every time we call RunTask() if the task needs a high resolution timer.
  int pending_high_res_tasks_;
  // Tracks if we have requested high resolution timers. Its only use is to
  // turn off the high resolution timer upon loop destruction.
  bool in_high_res_mode_;
#endif

  // Contains delayed tasks, sorted by their 'delayed_run_time' property.
  DelayedTaskQueue delayed_work_queue_;

  // A recent snapshot of Time::Now(), used to check delayed_work_queue_.
  TimeTicks recent_time_;

  // A queue of non-nestable tasks that we had to defer because when it came
  // time to execute them we were in a nested run loop.  They will execute
  // once we're out of nested run loops.
  TaskQueue deferred_non_nestable_work_queue_;

  ObserverList<DestructionObserver> destruction_observers_;

  // A recursion block that prevents accidentally running additional tasks when
  // insider a (accidentally induced?) nested message pump.
  bool nestable_tasks_allowed_;

  // pump_factory_.Run() is called to create a message pump for this loop
  // if type_ is TYPE_CUSTOM and pump_ is null.
  MessagePumpFactoryCallback pump_factory_;

  ObserverList<TaskObserver> task_observers_;

  // Used to allow creating a breadcrumb of program counters in PostTask.
  // This variable is only initialized while a task is being executed and is
  // meant only to store context for creating a backtrace breadcrumb. Do not
  // attach other semantics to it without thinking through the use caes
  // thoroughly.
  const PendingTask* current_pending_task_;

  RefPtr<internal::IncomingTaskQueue> incoming_task_queue_;

  // A task runner which we haven't bound to a thread yet.
  RefPtr<internal::MessageLoopTaskRunner> unbound_task_runner_;

  // The task runner associated with this message loop.
  RefPtr<SingleThreadTaskRunner> task_runner_;
  std::unique_ptr<ThreadTaskRunnerHandle> thread_task_runner_handle_;

  // Id of the thread this message loop is bound to. Initialized once when the
  // MessageLoop is bound to its thread and constant forever after.
  PlatformThreadId thread_id_;

  // Whether task observers are allowed.
  bool allow_task_observers_ = true;

  // An interface back to RunLoop state accessible by this RunLoop::Delegate.
  RunLoop::Delegate::Client* run_loop_client_ = nullptr;
};

//-----------------------------------------------------------------------------
// MessageLoopForUI extends MessageLoop with methods that are particular to a
// MessageLoop instantiated with TYPE_UI.
//
// This class is typically used like so:
//   MessageLoopForUI::current()->...call some method...
//
class CRBASE_EXPORT MessageLoopForUI : public MessageLoop {
 public:
  MessageLoopForUI() : MessageLoop(TYPE_UI) {
  }

  explicit MessageLoopForUI(std::unique_ptr<MessagePump> pump);

  // Returns the MessageLoopForUI of the current thread.
  static MessageLoopForUI* current() {
    MessageLoop* loop = MessageLoop::current();
    CR_DCHECK(loop);
    CR_DCHECK(loop->IsType(MessageLoop::TYPE_UI));
    return static_cast<MessageLoopForUI*>(loop);
  }

  static bool IsCurrent() {
    MessageLoop* loop = MessageLoop::current();
    return loop && loop->IsType(MessageLoop::TYPE_UI);
  }

#if defined(MINI_CHROMIUM_OS_LINUX)
  // Please see MessagePumpLibevent for definition.
  bool WatchFileDescriptor(
      int fd,
      bool persistent,
      MessagePumpLibevent::Mode mode,
      MessagePumpLibevent::FileDescriptorWatcher* controller,
      MessagePumpLibevent::Watcher* delegate);
#endif
};

// Do not add any member variables to MessageLoopForUI!  This is important b/c
// MessageLoopForUI is often allocated via MessageLoop(TYPE_UI).  Any extra
// data that you need should be stored on the MessageLoop's pump_ instance.
static_assert(sizeof(MessageLoop) == sizeof(MessageLoopForUI),
              "MessageLoopForUI should not have extra member variables");

//-----------------------------------------------------------------------------
// MessageLoopForIO extends MessageLoop with methods that are particular to a
// MessageLoop instantiated with TYPE_IO.
//
// This class is typically used like so:
//   MessageLoopForIO::current()->...call some method...
//
class CRBASE_EXPORT MessageLoopForIO : public MessageLoop {
 public:
  MessageLoopForIO() : MessageLoop(TYPE_IO) {
  }

  // Returns the MessageLoopForIO of the current thread.
  static MessageLoopForIO* current() {
    MessageLoop* loop = MessageLoop::current();
    CR_DCHECK(loop);
    CR_DCHECK(MessageLoop::TYPE_IO == loop->type());
    return static_cast<MessageLoopForIO*>(loop);
  }

  static bool IsCurrent() {
    MessageLoop* loop = MessageLoop::current();
    return loop && loop->type() == MessageLoop::TYPE_IO;
  }

#if defined(MINI_CHROMIUM_OS_WIN)
  typedef MessagePumpForIO::IOHandler IOHandler;
  typedef MessagePumpForIO::IOContext IOContext;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  typedef MessagePumpLibevent::Watcher Watcher;
  typedef MessagePumpLibevent::FileDescriptorWatcher
      FileDescriptorWatcher;

  enum Mode {
    WATCH_READ = MessagePumpLibevent::WATCH_READ,
    WATCH_WRITE = MessagePumpLibevent::WATCH_WRITE,
    WATCH_READ_WRITE = MessagePumpLibevent::WATCH_READ_WRITE
  };
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
  // Please see MessagePumpWin for definitions of these methods.
  void RegisterIOHandler(HANDLE file, IOHandler* handler);
  bool RegisterJobObject(HANDLE job, IOHandler* handler);
  bool WaitForIOCompletion(DWORD timeout, IOHandler* filter);
#elif defined(MINI_CHROMIUM_OS_POSIX)
  // Please see MessagePumpIOSForIO/MessagePumpLibevent for definition.
  bool WatchFileDescriptor(int fd,
                           bool persistent,
                           Mode mode,
                           FileDescriptorWatcher* controller,
                           Watcher* delegate);
#endif  // defined(MINI_CHROMIUM_OS_WIN)
};

// Do not add any member variables to MessageLoopForIO!  This is important b/c
// MessageLoopForIO is often allocated via MessageLoop(TYPE_IO).  Any extra
// data that you need should be stored on the MessageLoop's pump_ instance.
static_assert(sizeof(MessageLoop) == sizeof(MessageLoopForIO),
              "MessageLoopForIO should not have extra member variables");

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_MESSAGE_LOOP_H_

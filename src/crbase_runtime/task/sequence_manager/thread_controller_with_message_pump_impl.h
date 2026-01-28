// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_WITH_MESSAGE_PUMP_IMPL_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_WITH_MESSAGE_PUMP_IMPL_H_

#include <memory>

#include "crbase/containers/optional.h"
#include "crbase/threading/platform_thread.h"
#include "crbase_runtime/message_pump/message_pump.h"
#include "crbase_runtime/message_pump/work_id_provider.h"
#include "crbase_runtime/run_loop.h"
#include "crbase_runtime/task/common/checked_lock.h"
#include "crbase_runtime/task/sequence_manager/associated_thread_id.h"
#include "crbase_runtime/task/sequence_manager/sequence_manager_impl.h"
#include "crbase_runtime/task/sequence_manager/sequenced_task_source.h"
#include "crbase_runtime/task/sequence_manager/thread_controller.h"
///#include "crbase_runtime/task/sequence_manager/thread_controller_power_monitor.h"
#include "crbase_runtime/task/sequence_manager/work_deduplicator.h"
//#include "crbase/threading/hang_watcher.h"
#include "crbase_runtime/threading/sequence_local_storage_map.h"
#include "crbase_runtime/threading/thread_task_runner_handle.h"
#include "crbuild/build_config.h"

namespace cr {
namespace sequence_manager {
namespace internal {

// This is the interface between the SequenceManager and the MessagePump.
class CRBASE_EXPORT ThreadControllerWithMessagePumpImpl
    : public ThreadController,
      public MessagePump::Delegate,
      public RunLoop::Delegate,
      public RunLoop::NestingObserver {
 public:
  ThreadControllerWithMessagePumpImpl(
      std::unique_ptr<MessagePump> message_pump,
      const SequenceManager::Settings& settings);
  ThreadControllerWithMessagePumpImpl(
      const ThreadControllerWithMessagePumpImpl&) = delete;
  ThreadControllerWithMessagePumpImpl& operator=(
      const ThreadControllerWithMessagePumpImpl&) = delete;
  ~ThreadControllerWithMessagePumpImpl() override;

  using ShouldScheduleWork = WorkDeduplicator::ShouldScheduleWork;

  static std::unique_ptr<ThreadControllerWithMessagePumpImpl> CreateUnbound(
      const SequenceManager::Settings& settings);

  // ThreadController implementation:
  void SetSequencedTaskSource(SequencedTaskSource* task_source) override;
  void BindToCurrentThread(std::unique_ptr<MessagePump> message_pump) override;
  void SetWorkBatchSize(size_t work_batch_size) override;
  void WillQueueTask(PendingTask* pending_task,
                     const char* task_queue_name) override;
  void ScheduleWork() override;
  void SetNextDelayedDoWork(LazyNow* lazy_now, TimeTicks run_time) override;
  void SetTimerSlack(TimerSlack timer_slack) override;
  const TickClock* GetClock() override;
  bool RunsTasksInCurrentSequence() override;
  void SetDefaultTaskRunner(
      RefPtr<SingleThreadTaskRunner> task_runner) override;
  RefPtr<SingleThreadTaskRunner> GetDefaultTaskRunner() override;
  void RestoreDefaultTaskRunner() override;
  void AddNestingObserver(RunLoop::NestingObserver* observer) override;
  void RemoveNestingObserver(RunLoop::NestingObserver* observer) override;
  const RefPtr<AssociatedThreadId>& GetAssociatedThread() const override;
  void SetTaskExecutionAllowed(bool allowed) override;
  bool IsTaskExecutionAllowed() const override;
  MessagePump* GetBoundMessagePump() const override;
  bool ShouldQuitRunLoopWhenIdle() override;

  // RunLoop::NestingObserver:
  void OnBeginNestedRunLoop() override;
  void OnExitNestedRunLoop() override;

 protected:
  explicit ThreadControllerWithMessagePumpImpl(
      const SequenceManager::Settings& settings);

  // MessagePump::Delegate implementation.
  void OnBeginNativeWork() override;
  void OnEndNativeWork() override;
  void BeforeWait() override;
  MessagePump::Delegate::NextWorkInfo DoWork() override;
  bool DoIdleWork() override;

  // RunLoop::Delegate implementation.
  void Run(bool application_tasks_allowed, TimeDelta timeout) override;
  void Quit() override;
  void EnsureWorkScheduled() override;

  struct MainThreadOnly {
    MainThreadOnly();
    ~MainThreadOnly();

    SequencedTaskSource* task_source = nullptr;            // Not owned.
    RunLoop::NestingObserver* nesting_observer = nullptr;  // Not owned.
    std::unique_ptr<ThreadTaskRunnerHandle> thread_task_runner_handle;

    // Indicates that we should yield DoWork between each task to let a possibly
    // nested RunLoop exit.
    bool quit_pending = false;

    // Whether high resolution timing is enabled or not.
    bool in_high_res_mode = false;

    // Number of tasks processed in a single DoWork invocation.
    size_t work_batch_size = 1;

    // Tracks the number and state of each run-level managed by this instance.
    RunLevelTracker run_level_tracker;

    // When the next scheduled delayed work should run, if any.
    TimeTicks next_delayed_do_work = TimeTicks::Max();

    // The time after which the runloop should quit.
    TimeTicks quit_runloop_after = TimeTicks::Max();

    bool task_execution_allowed = true;
  };

  const MainThreadOnly& MainThreadOnlyForTesting() const {
    return main_thread_only_;
  }

  ///ThreadControllerPowerMonitor* ThreadControllerPowerMonitorForTesting() {
  ///  return &power_monitor_;
  ///}

 private:
  friend class DoWorkScope;
  friend class RunScope;

  // Returns the delay till the next task. If there's no delay TimeDelta::Max()
  // will be returned.
  TimeDelta DoWorkImpl(LazyNow* continuation_lazy_now);

  void InitializeThreadTaskRunnerHandle();

  MainThreadOnly& main_thread_only() {
    CR_DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
    return main_thread_only_;
  }

  const MainThreadOnly& main_thread_only() const {
    CR_DCHECK_CALLED_ON_VALID_THREAD(associated_thread_->thread_checker);
    return main_thread_only_;
  }

  // Instantiate a HangWatchScopeEnabled to cover the current work if hang
  // watching is activated via finch and the current loop is not nested.
  void MaybeStartHangWatchScopeEnabled();

  // TODO(altimin): Merge with the one in SequenceManager.
  RefPtr<AssociatedThreadId> associated_thread_;
  MainThreadOnly main_thread_only_;

  mutable cr::internal::CheckedLock task_runner_lock_;
  RefPtr<SingleThreadTaskRunner> task_runner_
      /* GUARDED_BY(task_runner_lock_) */;

  WorkDeduplicator work_deduplicator_;

  ///ThreadControllerPowerMonitor power_monitor_;

  // Can only be set once (just before calling
  // work_deduplicator_.BindToCurrentThread()). After that only read access is
  // allowed.
  std::unique_ptr<MessagePump> pump_;

  const TickClock* time_source_;  // Not owned.

  // Non-null provider of id state for identifying distinct work items executed
  // by the message loop (task, event, etc.). Cached on the class to avoid TLS
  // lookups on task execution.
  WorkIdProvider* work_id_provider_ = nullptr;

  // Required to register the current thread as a sequence.
  cr::internal::SequenceLocalStorageMap sequence_local_storage_map_;
  std::unique_ptr<
      cr::internal::ScopedSetSequenceLocalStorageMapForCurrentThread>
      scoped_set_sequence_local_storage_map_for_current_thread_;

  // Reset at the start of each unit of work to cover the work itself and then
  // transition to the next one.
  ///cr::Optional<HangWatchScopeEnabled> hang_watch_scope_;
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_THREAD_CONTROLLER_WITH_MESSAGE_PUMP_IMPL_H_

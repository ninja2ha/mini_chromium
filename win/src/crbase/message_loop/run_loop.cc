// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/message_loop/run_loop.h"

#include "crbase/functional/bind.h"
#include "crbase/functional/callback.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/task/single_thread_task_runner.h"
#include "crbase/threading/thread_local.h"
#include "crbase/threading/thread_task_runner_handle.h"
#include "crbase/build_platform.h"

namespace cr {

namespace {
  
ThreadLocalPointer<RunLoop::Delegate>& GetTlsDelegate() {
  static cr::NoDestructor<ThreadLocalPointer<RunLoop::Delegate>> instance;
  return *instance;
}
// Runs |closure| immediately if this is called on |task_runner|, otherwise
// forwards |closure| to it.
void ProxyToTaskRunner(RefPtr<SequencedTaskRunner> task_runner,
                       OnceClosure closure) {
  if (task_runner->RunsTasksInCurrentSequence()) {
    std::move(closure).Run();
    return;
  }
  task_runner->PostTask(CR_FROM_HERE, std::move(closure));
}

}  // namespace

RunLoop::Delegate::Delegate() {
  // The Delegate can be created on another thread. It is only bound in
  // RegisterDelegateForCurrentThread().
  CR_DETACH_FROM_THREAD(bound_thread_checker_);
}

RunLoop::Delegate::~Delegate() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(bound_thread_checker_);
  // A RunLoop::Delegate may be destroyed before it is bound, if so it may still
  // be on its creation thread (e.g. a Thread that fails to start) and
  // shouldn't disrupt that thread's state.
  if (bound_)
    GetTlsDelegate().Set(nullptr);
}

RunLoop* RunLoop::Delegate::Client::GetTopMostRunLoop() const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(outer_->bound_thread_checker_);
  CR_DCHECK(outer_->bound_);
  return outer_->active_run_loops_.empty() ? nullptr
                                           : outer_->active_run_loops_.top();
}

bool RunLoop::Delegate::Client::IsNested() const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(outer_->bound_thread_checker_);
  CR_DCHECK(outer_->bound_);
  return outer_->active_run_loops_.size() > 1;
}

RunLoop::Delegate::Client::Client(Delegate* outer) : outer_(outer) {}

// static
RunLoop::Delegate::Client* RunLoop::RegisterDelegateForCurrentThread(
    Delegate* delegate) {
  // Bind |delegate| to this thread.
  CR_DCHECK(!delegate->bound_);
  CR_DCHECK_CALLED_ON_VALID_THREAD(delegate->bound_thread_checker_);

  // There can only be one RunLoop::Delegate per thread.
  CR_DCHECK(!GetTlsDelegate().Get());
  GetTlsDelegate().Set(delegate);
  delegate->bound_ = true;

  return &delegate->client_interface_;
}

RunLoop::RunLoop()
    : delegate_(GetTlsDelegate().Get()),
      origin_task_runner_(ThreadTaskRunnerHandle::Get()),
      weak_factory_(this) {
  // A RunLoop::Delegate must be bound to this thread prior to using RunLoop.
  CR_DCHECK(delegate_);
  CR_DCHECK(origin_task_runner_);
}

RunLoop::~RunLoop() {
  // TODO(gab): Fix bad usage and enable this check, http://crbug.com/715235.
  // DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void RunLoop::Run() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!BeforeRun())
    return;

  // It is okay to access this RunLoop from another sequence while Run() is
  // active as this RunLoop won't touch its state until after that returns (if
  // the RunLoop's state is accessed while processing Run(), it will be re-bound
  // to the accessing sequence for the remainder of that Run() -- accessing from
  // multiple sequences is still disallowed).
  CR_DETACH_FROM_SEQUENCE(sequence_checker_);

  // Use task stopwatch to exclude the loop run time from the current task, if
  // any.
  ///tracked_objects::TaskStopwatch stopwatch;
  ///stopwatch.Start();
  delegate_->Run();
  ///stopwatch.Stop();

  // Rebind this RunLoop to the current thread after Run().
  CR_DETACH_FROM_SEQUENCE(sequence_checker_);
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  AfterRun();
}

void RunLoop::RunUntilIdle() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  quit_when_idle_received_ = true;
  Run();
}

void RunLoop::Quit() {
  // Thread-safe.

  // This can only be hit if run_loop->Quit() is called directly (QuitClosure()
  // proxies through ProxyToTaskRunner() as it can only deref its WeakPtr on
  // |origin_task_runner_|).
  if (!origin_task_runner_->RunsTasksInCurrentSequence()) {
    origin_task_runner_->PostTask(CR_FROM_HERE,
                                  cr::BindOnce(&RunLoop::Quit, Unretained(this)));
    return;
  }

  quit_called_ = true;
  if (running_ && delegate_->active_run_loops_.top() == this) {
    // This is the inner-most RunLoop, so quit now.
    delegate_->Quit();
  }
}

void RunLoop::QuitWhenIdle() {
  // Thread-safe.

  // This can only be hit if run_loop->QuitWhenIdle() is called directly
  // (QuitWhenIdleClosure() proxies through ProxyToTaskRunner() as it can only
  // deref its WeakPtr on |origin_task_runner_|).
  if (!origin_task_runner_->RunsTasksInCurrentSequence()) {
    origin_task_runner_->PostTask(
        CR_FROM_HERE, cr::BindOnce(&RunLoop::QuitWhenIdle, Unretained(this)));
    return;
  }

  quit_when_idle_received_ = true;
}

cr::RepeatingClosure RunLoop::QuitClosure() {
  // TODO(gab): Fix bad usage and enable this check, http://crbug.com/715235.
  // DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Need to use ProxyToTaskRunner() as WeakPtrs vended from
  // |weak_factory_| may only be accessed on |origin_task_runner_|.
  // TODO(gab): It feels wrong that QuitClosure() is bound to a WeakPtr.
  return cr::BindRepeating(
      &ProxyToTaskRunner, origin_task_runner_,
      cr::BindRepeating(&RunLoop::Quit, weak_factory_.GetWeakPtr()));
}

cr::RepeatingClosure RunLoop::QuitWhenIdleClosure() {
  // TODO(gab): Fix bad usage and enable this check, http://crbug.com/715235.
  // DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Need to use ProxyToTaskRunner() as WeakPtrs vended from
  // |weak_factory_| may only be accessed on |origin_task_runner_|.
  // TODO(gab): It feels wrong that QuitWhenIdleClosure() is bound to a WeakPtr.
  return cr::BindRepeating(
      &ProxyToTaskRunner, origin_task_runner_,
      cr::BindRepeating(&RunLoop::QuitWhenIdle, weak_factory_.GetWeakPtr()));
}

// static
bool RunLoop::IsRunningOnCurrentThread() {
  Delegate* delegate = GetTlsDelegate().Get();
  return delegate && !delegate->active_run_loops_.empty();
}

// static
bool RunLoop::IsNestedOnCurrentThread() {
  Delegate* delegate = GetTlsDelegate().Get();
  return delegate && delegate->active_run_loops_.size() > 1;
}

// static
void RunLoop::AddNestingObserverOnCurrentThread(NestingObserver* observer) {
  Delegate* delegate = GetTlsDelegate().Get();
  CR_DCHECK(delegate);
  CR_CHECK(delegate->allow_nesting_);
  delegate->nesting_observers_.AddObserver(observer);
}

// static
void RunLoop::RemoveNestingObserverOnCurrentThread(NestingObserver* observer) {
  Delegate* delegate = GetTlsDelegate().Get();
  CR_DCHECK(delegate);
  CR_CHECK(delegate->allow_nesting_);
  delegate->nesting_observers_.RemoveObserver(observer);
}

// static
bool RunLoop::IsNestingAllowedOnCurrentThread() {
  return GetTlsDelegate().Get()->allow_nesting_;
}

// static
void RunLoop::DisallowNestingOnCurrentThread() {
  GetTlsDelegate().Get()->allow_nesting_ = false;
}

bool RunLoop::BeforeRun() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

#if CR_DCHECK_IS_ON()
  CR_DCHECK(!run_called_);
  run_called_ = true;
#endif  // DCHECK_IS_ON()

  // Allow Quit to be called before Run.
  if (quit_called_)
    return false;

  auto& active_run_loops_ = delegate_->active_run_loops_;
  active_run_loops_.push(this);

  const bool is_nested = active_run_loops_.size() > 1;

  if (is_nested) {
    CR_CHECK(delegate_->allow_nesting_);
    for (auto& observer : delegate_->nesting_observers_)
      observer.OnBeginNestedRunLoop();
  }

  running_ = true;
  return true;
}

void RunLoop::AfterRun() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  running_ = false;

  auto& active_run_loops_ = delegate_->active_run_loops_;
  CR_DCHECK(active_run_loops_.top() == this);
  active_run_loops_.pop();

  RunLoop* previous_run_loop =
      active_run_loops_.empty() ? nullptr : active_run_loops_.top();

  // Execute deferred QuitNow, if any:
  if (previous_run_loop && previous_run_loop->quit_called_)
    delegate_->Quit();
}

}  // namespace cr

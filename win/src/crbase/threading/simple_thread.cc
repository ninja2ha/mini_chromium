// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/simple_thread.h"

#include "crbase/logging.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/threading/platform_thread.h"
#include "crbase/threading/thread_restrictions.h"

namespace cr {

SimpleThread::SimpleThread(const std::string& name_prefix)
    : SimpleThread(name_prefix, Options()) {}

SimpleThread::SimpleThread(const std::string& name_prefix,
                           const Options& options)
    : name_prefix_(name_prefix),
      options_(options),
      event_(WaitableEvent::ResetPolicy::MANUAL,
             WaitableEvent::InitialState::NOT_SIGNALED) {}

SimpleThread::~SimpleThread() {
  CR_DCHECK(HasBeenStarted()) << "SimpleThread was never started.";
  CR_DCHECK(!options_.joinable || HasBeenJoined())
      << "Joinable SimpleThread destroyed without being Join()ed.";
}

void SimpleThread::Start() {
  CR_DCHECK(!HasBeenStarted()) << "Tried to Start a thread multiple times.";
  bool success =
      options_.joinable
          ? PlatformThread::CreateWithPriority(options_.stack_size, this,
                                               &thread_, options_.priority)
          : PlatformThread::CreateNonJoinableWithPriority(
                options_.stack_size, this, options_.priority);
  CR_DCHECK(success);
  ThreadRestrictions::ScopedAllowWait allow_wait;
  event_.Wait();  // Wait for the thread to complete initialization.
}

void SimpleThread::Join() {
  CR_DCHECK(options_.joinable) << "A non-joinable thread can't be joined.";
  CR_DCHECK(HasBeenStarted()) << "Tried to Join a never-started thread.";
  CR_DCHECK(!HasBeenJoined()) << "Tried to Join a thread multiple times.";
  PlatformThread::Join(thread_);
  thread_ = PlatformThreadHandle();
  joined_ = true;
}

bool SimpleThread::HasBeenStarted() {
  ThreadRestrictions::ScopedAllowWait allow_wait;
  return event_.IsSignaled();
}

void SimpleThread::ThreadMain() {
  tid_ = PlatformThread::CurrentId();
  // Construct our full name of the form "name_prefix_/TID".
  std::string name(name_prefix_);
  name.push_back('/');
  name.append(NumberToString(tid_));
  PlatformThread::SetName(name);

  // We've initialized our new thread, signal that we're done to Start().
  event_.Signal();

  Run();
}

DelegateSimpleThread::DelegateSimpleThread(Delegate* delegate,
                                           const std::string& name_prefix)
    : DelegateSimpleThread(delegate, name_prefix, Options()) {}

DelegateSimpleThread::DelegateSimpleThread(Delegate* delegate,
                                           const std::string& name_prefix,
                                           const Options& options)
    : SimpleThread(name_prefix, options),
      delegate_(delegate) {
  CR_DCHECK(delegate_);
}

DelegateSimpleThread::~DelegateSimpleThread() = default;

void DelegateSimpleThread::Run() {
  CR_DCHECK(delegate_) << "Tried to call Run without a delegate (called twice?)";

  // Non-joinable DelegateSimpleThreads are allowed to be deleted during Run().
  // Member state must not be accessed after invoking Run().
  Delegate* delegate = delegate_;
  delegate_ = nullptr;
  delegate->Run();
}

DelegateSimpleThreadPool::DelegateSimpleThreadPool(
    const std::string& name_prefix,
    int num_threads)
    : name_prefix_(name_prefix),
      num_threads_(num_threads),
      dry_(WaitableEvent::ResetPolicy::MANUAL,
           WaitableEvent::InitialState::NOT_SIGNALED) {}

DelegateSimpleThreadPool::~DelegateSimpleThreadPool() {
  CR_DCHECK(threads_.empty());
  CR_DCHECK(delegates_.empty());
  CR_DCHECK(!dry_.IsSignaled());
}

void DelegateSimpleThreadPool::Start() {
  CR_DCHECK(threads_.empty()) << "Start() called with outstanding threads.";
  for (int i = 0; i < num_threads_; ++i) {
    DelegateSimpleThread* thread = new DelegateSimpleThread(this, name_prefix_);
    thread->Start();
    threads_.push_back(thread);
  }
}

void DelegateSimpleThreadPool::JoinAll() {
  CR_DCHECK(!threads_.empty()) 
      << "JoinAll() called with no outstanding threads.";

  // Tell all our threads to quit their worker loop.
  AddWork(NULL, num_threads_);

  // Join and destroy all the worker threads.
  for (int i = 0; i < num_threads_; ++i) {
    threads_[i]->Join();
    delete threads_[i];
  }
  threads_.clear();
  CR_DCHECK(delegates_.empty());
}

void DelegateSimpleThreadPool::AddWork(Delegate* delegate, int repeat_count) {
  AutoLock locked(lock_);
  for (int i = 0; i < repeat_count; ++i)
    delegates_.push(delegate);
  // If we were empty, signal that we have work now.
  if (!dry_.IsSignaled())
    dry_.Signal();
}

void DelegateSimpleThreadPool::Run() {
  Delegate* work = NULL;

  while (true) {
    dry_.Wait();
    {
      AutoLock locked(lock_);
      if (!dry_.IsSignaled())
        continue;

      CR_DCHECK(!delegates_.empty());
      work = delegates_.front();
      delegates_.pop();

      // Signal to any other threads that we're currently out of work.
      if (delegates_.empty())
        dry_.Reset();
    }

    // A NULL delegate pointer signals us to quit.
    if (!work)
      break;

    work->Run();
  }
}

}  // namespace cr
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_THREADING_THREAD_RESTRICTIONS_H_
#define MINI_CHROMIUM_CRBASE_THREADING_THREAD_RESTRICTIONS_H_

#include "crbase/base_export.h"
#include "crbase/logging.h"

namespace cr {

class SequencedWorkerPool;
class SimpleThread;
class Thread;

// Certain behavior is disallowed on certain threads.  ThreadRestrictions helps
// enforce these rules.  Examples of such rules:
//
// * Do not do blocking IO (makes the thread janky)
// * Do not access Singleton/LazyInstance (may lead to shutdown crashes)
//
// Here's more about how the protection works:
//
// 1) If a thread should not be allowed to make IO calls, mark it:
//      cr::ThreadRestrictions::SetIOAllowed(false);
//    By default, threads *are* allowed to make IO calls.
//    In Chrome browser code, IO calls should be proxied to the File thread.
//
// 2) If a function makes a call that will go out to disk, check whether the
//    current thread is allowed:
//      cr::ThreadRestrictions::AssertIOAllowed();
//
//
// Style tip: where should you put AssertIOAllowed checks?  It's best
// if you put them as close to the disk access as possible, at the
// lowest level.  This rule is simple to follow and helps catch all
// callers.  For example, if your function GoDoSomeBlockingDiskCall()
// only calls other functions in Chrome and not fopen(), you should go
// add the AssertIOAllowed checks in the helper functions.

class CRBASE_EXPORT ThreadRestrictions {
 public:
  ThreadRestrictions(const ThreadRestrictions&) = delete;
  ThreadRestrictions& operator=(const ThreadRestrictions&) = delete;

  // Constructing a ScopedAllowIO temporarily allows IO for the current
  // thread.  Doing this is almost certainly always incorrect.
  class CRBASE_EXPORT ScopedAllowIO {
   public:
    ScopedAllowIO(const ScopedAllowIO&) = delete;
    ScopedAllowIO& operator=(const ScopedAllowIO&) = delete;

    ScopedAllowIO() { previous_value_ = SetIOAllowed(true); }
    ~ScopedAllowIO() { SetIOAllowed(previous_value_); }
   private:
    // Whether IO is allowed when the ScopedAllowIO was constructed.
    bool previous_value_;
  };

#if CR_DCHECK_IS_ON()
  // Set whether the current thread to make IO calls.
  // Threads start out in the *allowed* state.
  // Returns the previous value.
  static bool SetIOAllowed(bool allowed);

  // Check whether the current thread is allowed to make IO calls,
  // and DCHECK if not.  See the block comment above the class for
  // a discussion of where to add these checks.
  static void AssertIOAllowed();

  // Set whether the current thread can use singletons.  Returns the previous
  // value.
  static bool SetSingletonAllowed(bool allowed);

  // Check whether the current thread is allowed to use singletons (Singleton /
  // LazyInstance).  DCHECKs if not.
  static void AssertSingletonAllowed();

  // Disable waiting on the current thread. Threads start out in the *allowed*
  // state. Returns the previous value.
  static void DisallowWaiting();

  // Check whether the current thread is allowed to wait, and DCHECK if not.
  static void AssertWaitAllowed();
#else
  // Inline the empty definitions of these functions so that they can be
  // compiled out.
  static bool SetIOAllowed(bool allowed) { return true; }
  static void AssertIOAllowed() {}
  static bool SetSingletonAllowed(bool allowed) { return true; }
  static void AssertSingletonAllowed() {}
  static void DisallowWaiting() {}
  static void AssertWaitAllowed() {}
#endif

 private:
  // DO NOT ADD ANY OTHER FRIEND STATEMENTS, talk to jam or brettw first.
  // BEGIN ALLOWED USAGE.
  friend class MessagePumpDefault;
  friend class SequencedWorkerPool;
  friend class SimpleThread;
  friend class Thread;
  friend class PlatformThread;
  // END ALLOWED USAGE.

#if CR_DCHECK_IS_ON()
  static bool SetWaitAllowed(bool allowed);
#else
  static bool SetWaitAllowed(bool allowed) { return true; }
#endif

  // Constructing a ScopedAllowWait temporarily allows waiting on the current
  // thread.  Doing this is almost always incorrect, which is why we limit who
  // can use this through friend. If you find yourself needing to use this, find
  // another way. Talk to jam or brettw.
  class CRBASE_EXPORT ScopedAllowWait {
   public:
    ScopedAllowWait(const ScopedAllowWait&) = delete;
    ScopedAllowWait& operator=(const ScopedAllowWait&) = delete;

    ScopedAllowWait() { previous_value_ = SetWaitAllowed(true); }
    ~ScopedAllowWait() { SetWaitAllowed(previous_value_); }
   private:
    // Whether singleton use is allowed when the ScopedAllowWait was
    // constructed.
    bool previous_value_;
  };
};

}  // namespace cr

#endif  // MINI_CHROMIUM_CRBASE_THREADING_THREAD_RESTRICTIONS_H_
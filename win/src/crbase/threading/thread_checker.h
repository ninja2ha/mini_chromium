// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_CHECKER_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_CHECKER_H_

#include "crbase/compiler_specific.h"
#include "crbase/logging.h"
#include "crbase/strings/string_piece.h"
#include "crbase/threading/thread_checker_impl.h"

// ThreadChecker is a helper class used to help verify that some methods of a
// class are called from the same thread (for thread-affinity).  It supports
// thread safety annotations (see base/thread_annotations.h).
//
// Use the macros below instead of the ThreadChecker directly so that the unused
// member doesn't result in an extra byte (four when padded) per instance in
// production.
//
// Usage of this class should be *rare* as most classes require thread-safety
// but not thread-affinity. Prefer base::SequenceChecker to verify thread-safe
// access.
//
// Thread-affinity checks should only be required in classes that use thread-
// local-storage or a third-party API that does.
//
// Prefer to encode the minimum requirements of each class instead of the
// environment it happens to run in today. e.g. if a class requires thread-
// safety but not thread-affinity, use a SequenceChecker even if it happens to
// run on a SingleThreadTaskRunner today. That makes it easier to understand
// what would need to change to turn that SingleThreadTaskRunner into a
// SequencedTaskRunner for ease of scheduling as well as minimizes side-effects
// if that change is made.
//
// Usage:
//   class MyClass {
//    public:
//     MyClass() {
//       // It's sometimes useful to detach on construction for objects that are
//       // constructed in one place and forever after used from another
//       // thread.
//       CR_DETACH_FROM_THREAD(thread_checker_);
//     }
//
//     ~MyClass() {
//       // ThreadChecker doesn't automatically check it's destroyed on origin
//       // thread for the same reason it's sometimes detached in the
//       // constructor. It's okay to destroy off thread if the owner otherwise
//       // knows usage on the associated thread is done. If you're not
//       // detaching in the constructor, you probably want to explicitly check
//       // in the destructor.
//       CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
//     }
//
//     void MyMethod() {
//       CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
//       ... (do stuff) ...
//     }
//
//     void MyOtherMethod() {
//       foo_ = 42;
//     }
//
//    private:
//     int foo_;
//
//     CR_THREAD_CHECKER(thread_checker_);
//   }

#if CR_DCHECK_IS_ON()
#define CR_THREAD_CHECKER(name) cr::ThreadChecker name;
#define CR_DCHECK_CALLED_ON_VALID_THREAD(name) \
    CR_DCHECK((name).CalledOnValidThread())
#define CR_DETACH_FROM_THREAD(name) (name).DetachFromThread()
#else  // CR_DCHECK_IS_ON()
#define CR_THREAD_CHECKER(name)
#define CR_DCHECK_CALLED_ON_VALID_THREAD(name)
#define CR_DETACH_FROM_THREAD(name)
#endif  // CR_DCHECK_IS_ON()

namespace cr {

// Do nothing implementation, for use in release mode.
//
// Note: You should almost always use the ThreadChecker class (through the above
// macros) to get the right version for your build configuration.
// Note: This is only a check, not a "lock". It is marked "LOCKABLE" only in
// order to support thread_annotations.h.
class ThreadCheckerDoNothing {
 public:
  ThreadCheckerDoNothing(const ThreadCheckerDoNothing&) = delete;
  ThreadCheckerDoNothing& operator=(const ThreadCheckerDoNothing&) = delete;
  ThreadCheckerDoNothing() = default;

  // Moving between matching threads is allowed to help classes with
  // ThreadCheckers that want a default move-construct/assign.
  ThreadCheckerDoNothing(ThreadCheckerDoNothing&& other) = default;
  ThreadCheckerDoNothing& operator=(ThreadCheckerDoNothing&& other) = default;

  bool CalledOnValidThread() const CR_WARN_UNUSED_RESULT { return true; }
  void DetachFromThread() {}
};

// Note that ThreadCheckerImpl::CalledOnValidThread() returns false when called
// from tasks posted to SingleThreadTaskRunners bound to different sequences,
// even if the tasks happen to run on the same thread (e.g. two independent
// SingleThreadTaskRunners on the ThreadPool that happen to share a thread).
#if CR_DCHECK_IS_ON()
class ThreadChecker : public ThreadCheckerImpl {
};
#else
class ThreadChecker : public ThreadCheckerDoNothing {
};
#endif  // DCHECK_IS_ON()

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_CHECKER_H_
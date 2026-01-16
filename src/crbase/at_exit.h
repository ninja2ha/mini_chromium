// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_AT_EXIT_H_
#define MINI_CHROMIUM_SRC_CRBASE_AT_EXIT_H_

#include "crbase/base_export.h"
#include "crbase/logging/logging.h"
#include "crbase/functional/callback.h"
#include "crbase/containers/stack.h"
#include "crbase/synchronization/lock.h"
///#include "crbase/thread_annotations.h"

namespace cr {

// This class provides a facility similar to the CRT atexit(), except that
// we control when the callbacks are executed. Under Windows for a DLL they
// happen at a really bad time and under the loader lock. This facility is
// mostly used by cr::Singleton.
//
// The usage is simple. Early in the main() or WinMain() scope create ann
// AtExitManager object on the stack:
// int main(...) {
//    cr::AtExitManager exit_manager;
//
// }
// When the exit_manager object goes out of scope, all the registered
// callbacks and singleton destructors will be called.

class CRBASE_EXPORT AtExitManager {
 public:
  typedef void (*AtExitCallbackType)(void*);

  AtExitManager();
  AtExitManager(const AtExitManager&) = delete;
  AtExitManager& operator=(const AtExitManager&) = delete;

  // The dtor calls all the registered callbacks. Do not try to register more
  // callbacks after this point. 
  ~AtExitManager();

  // Registers the specified function to be called at exit. The prototype of
  // the callback function is void func(void*).
  static void RegisterCallback(AtExitCallbackType func, void* param);

  // Registers the specified task to be called at exit.
  static void RegisterTask(cr::OnceClosure task);

  // Calls the functions registered with RegisterCallback in LIFO order. It
  // is possible to register new callbacks after calling this function.
  static void ProcessCallbacksNow();

  // Disable all registered at-exit callbacks. This is used only in a single-
  // process mode.
  static void DisableAllAtExitManagers();

 protected:
  // This constructor will allow this instance of AtExitManager to be created
  // even if one already exists.  This should only be used for testing!
  // AtExitManagers are kept on a global stack, and it will be removed during
  // destruction.  This allows you to shadow another AtExitManager.
  explicit AtExitManager(bool shadow);

 private:
  cr::Lock lock_;

  cr::Stack<cr::OnceClosure> stack_ /* GUARDED_BY(lock_) */;

#if CR_DCHECK_IS_ON()
  bool processing_callbacks_ /* GUARDED_BY(lock_) */ = false;
#endif

  // Stack of managers to allow shadowing.
  AtExitManager* const next_manager_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_AT_EXIT_H_

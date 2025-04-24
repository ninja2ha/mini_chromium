// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_OBJECT_WATCHER_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_OBJECT_WATCHER_H_

#include <windows.h>

#include "crbase/base_export.h"
#include "crbase/functional/callback.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/memory/weak_ptr.h"
#include "crbase/threading/task/sequenced_task_runner.h"

namespace cr {
namespace win {

// A class that provides a means to asynchronously wait for a Windows object to
// become signaled.  It is an abstraction around RegisterWaitForSingleObject
// that provides a notification callback, OnObjectSignaled, that runs back on
// the origin sequence (i.e., the sequence that called StartWatching).
//
// This class acts like a smart pointer such that when it goes out-of-scope,
// UnregisterWaitEx is automatically called, and any in-flight notification is
// suppressed.
//
// The waiting handle MUST NOT be closed while watching is in progress. If this
// handle is closed while the wait is still pending, the behavior is undefined
// (see MSDN:RegisterWaitForSingleObject).
//
// Typical usage:
//
//   class MyClass : public cr::win::ObjectWatcher::Delegate {
//    public:
//     void DoStuffWhenSignaled(HANDLE object) {
//       watcher_.StartWatchingOnce(object, this);
//     }
//     void OnObjectSignaled(HANDLE object) override {
//       // OK, time to do stuff!
//     }
//    private:
//     cr::win::ObjectWatcher watcher_;
//   };
//
// In the above example, MyClass wants to "do stuff" when object becomes
// signaled.  ObjectWatcher makes this task easy.  When MyClass goes out of
// scope, the watcher_ will be destroyed, and there is no need to worry about
// OnObjectSignaled being called on a deleted MyClass pointer.  Easy!
// If the object is already signaled before being watched, OnObjectSignaled is
// still called after (but not necessarily immediately after) watch is started.
//
// NOTE: Except for the constructor, all public methods of this class must be
// called in sequence, in a scope where SequencedTaskRunnerHandle::IsSet().
class CRBASE_EXPORT ObjectWatcher {
 public:
  class CRBASE_EXPORT Delegate {
   public:
    virtual ~Delegate() = default;
    // Called from the sequence that started the watch when a signaled object is
    // detected. To continue watching the object, StartWatching must be called
    // again.
    virtual void OnObjectSignaled(HANDLE object) = 0;
  };

  ObjectWatcher();
  ~ObjectWatcher();

  // When the object is signaled, the given delegate is notified on the sequence
  // where StartWatchingOnce is called. The ObjectWatcher is not responsible for
  // deleting the delegate.
  // Returns whether watching was successfully initiated.
  bool StartWatchingOnce(HANDLE object,
                         Delegate* delegate,
                         const Location& from_here);

  // Notifies the delegate, on the sequence where this method is called, each
  // time the object is set. By definition, the handle must be an auto-reset
  // object. The caller must ensure that it (or any Windows system code) doesn't
  // reset the event or else the delegate won't be called.
  // Returns whether watching was successfully initiated.
  bool StartWatchingMultipleTimes(
      HANDLE object,
      Delegate* delegate,
      const Location& from_here);

  // Stops watching.  Does nothing if the watch has already completed.  If the
  // watch is still active, then it is canceled, and the associated delegate is
  // not notified.
  //
  // Returns true if the watch was canceled.  Otherwise, false is returned.
  bool StopWatching();

  // Returns true if currently watching an object.
  bool IsWatching() const;

  // Returns the handle of the object being watched.
  HANDLE GetWatchedObject() const;

 private:
  // Called on a background thread when done waiting.
  static void CALLBACK DoneWaiting(void* param, BOOLEAN timed_out);

  // Helper used by StartWatchingOnce and StartWatchingMultipleTimes.
  bool StartWatchingInternal(HANDLE object,
                             Delegate* delegate,
                             bool execute_only_once,
                             const Location& from_here);

  void Signal(Delegate* delegate);

  void Reset();

  Location location_;

  // A callback pre-bound to Signal() that is posted to the caller's task runner
  // when the wait completes.
  RepeatingClosure callback_;

  // The object being watched.
  HANDLE object_ = nullptr;

  // The wait handle returned by RegisterWaitForSingleObject.
  HANDLE wait_object_ = nullptr;

  // The task runner of the sequence on which the watch was started.
  RefPtr<SequencedTaskRunner> task_runner_;

  bool run_once_ = true;

  WeakPtrFactory<ObjectWatcher> weak_factory_{this};
};

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_OBJECT_WATCHER_H_
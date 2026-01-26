// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_WAITABLE_EVENT_H_
#define MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_WAITABLE_EVENT_H_

#include <stddef.h>

#include "crbase/base_export.h"
#include "crbuild/compiler_specific.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/scoped_handle.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <list>
#include <utility>

#include "crbase/memory/ref_counted.h"
#include "crbase/synchronization/lock.h"
#endif

namespace cr {

class TimeDelta;

// A WaitableEvent can be a useful thread synchronization tool when you want to
// allow one thread to wait for another thread to finish some work. For
// non-Windows systems, this can only be used from within a single address
// space.
//
// Use a WaitableEvent when you would otherwise use a Lock+ConditionVariable to
// protect a simple boolean value.  However, if you find yourself using a
// WaitableEvent in conjunction with a Lock to wait for a more complex state
// change (e.g., for an item to be added to a queue), then you should probably
// be using a ConditionVariable instead of a WaitableEvent.
//
// NOTE: On Windows, this class provides a subset of the functionality afforded
// by a Windows event object.  This is intentional.  If you are writing Windows
// specific code and you need other features of a Windows event, then you might
// be better off just using an Windows event directly.
class CRBASE_EXPORT WaitableEvent {
 public:
  WaitableEvent(const WaitableEvent&) = delete;
  WaitableEvent& operator=(const WaitableEvent&) = delete;

  // Indicates whether a WaitableEvent should automatically reset the event
  // state after a single waiting thread has been released or remain signaled
  // until Reset() is manually invoked.
  enum class ResetPolicy { MANUAL, AUTOMATIC };

  // Indicates whether a new WaitableEvent should start in a signaled state or
  // not.
  enum class InitialState { SIGNALED, NOT_SIGNALED };

  // Constructs a WaitableEvent with policy and initial state as detailed in
  // the above enums.
  WaitableEvent(ResetPolicy reset_policy = ResetPolicy::MANUAL,
                InitialState initial_state = InitialState::NOT_SIGNALED);

#if defined(MINI_CHROMIUM_OS_WIN)
  // Create a WaitableEvent from an Event HANDLE which has already been
  // created. This objects takes ownership of the HANDLE and will close it when
  // deleted.
  explicit WaitableEvent(win::ScopedHandle event_handle);
#endif

  ~WaitableEvent();

  // Put the event in the un-signaled state.
  void Reset();

  // Put the event in the signaled state.  Causing any thread blocked on Wait
  // to be woken up.
  void Signal();

  // Returns true if the event is in the signaled state, else false.  If this
  // is not a manual reset event, then this test will cause a reset.
  bool IsSignaled();

  // Wait indefinitely for the event to be signaled. Wait's return "happens
  // after" |Signal| has completed. This means that it's safe for a
  // WaitableEvent to synchronise its own destruction, like this:
  //
  //   WaitableEvent *e = new WaitableEvent;
  //   SendToOtherThread(e);
  //   e->Wait();
  //   delete e;
  void Wait();

  // Wait up until wait_delta has passed for the event to be signaled
  // (real-time; ignores time overrides).  Returns true if the event was
  // signaled. Handles spurious wakeups and guarantees that |wait_delta| will
  // have elapsed if this returns false.
  //
  // TimedWait can synchronise its own destruction like |Wait|.
  bool TimedWait(const TimeDelta& wait_delta);

#if defined(MINI_CHROMIUM_OS_WIN)
  HANDLE handle() const { return handle_.Get(); }
#endif

  // Declares that this WaitableEvent will only ever be used by a thread that is
  // idle at the bottom of its stack and waiting for work (in particular, it is
  // not synchronously waiting on this event before resuming ongoing work). This
  // is useful to avoid telling base-internals that this thread is "blocked"
  // when it's merely idle and ready to do work. As such, this is only expected
  // to be used by thread and thread pool impls.
  ///void declare_only_used_while_idle() { waiting_is_blocking_ = false; }

  // Wait, synchronously, on multiple events.
  //   waitables: an array of WaitableEvent pointers
  //   count: the number of elements in @waitables
  //
  // returns: the index of a WaitableEvent which has been signaled.
  //
  // You MUST NOT delete any of the WaitableEvent objects while this wait is
  // happening, however WaitMany's return "happens after" the |Signal| call
  // that caused it has completed, like |Wait|.
  //
  // If more than one WaitableEvent is signaled to unblock WaitMany, the lowest
  // index among them is returned.
  static size_t WaitMany(WaitableEvent** waitables, size_t count);

  // For asynchronous waiting, see WaitableEventWatcher

  // This is a private helper class. It's here because it's used by friends of
  // this class (such as WaitableEventWatcher) to be able to enqueue elements
  // of the wait-list
  class Waiter {
   public:
    // Signal the waiter to wake up.
    //
    // Consider the case of a Waiter which is in multiple WaitableEvent's
    // wait-lists. Each WaitableEvent is automatic-reset and two of them are
    // signaled at the same time. Now, each will wake only the first waiter in
    // the wake-list before resetting. However, if those two waiters happen to
    // be the same object (as can happen if another thread didn't have a chance
    // to dequeue the waiter from the other wait-list in time), two auto-resets
    // will have happened, but only one waiter has been signaled!
    //
    // Because of this, a Waiter may "reject" a wake by returning false. In
    // this case, the auto-reset WaitableEvent shouldn't act as if anything has
    // been notified.
    virtual bool Fire(WaitableEvent* signaling_event) = 0;

    // Waiters may implement this in order to provide an extra condition for
    // two Waiters to be considered equal. In WaitableEvent::Dequeue, if the
    // pointers match then this function is called as a final check. See the
    // comments in ~Handle for why.
    virtual bool Compare(void* tag) = 0;

   protected:
    virtual ~Waiter() = default;
  };

 private:
  friend class WaitableEventWatcher;

#if defined(MINI_CHROMIUM_OS_WIN)
  win::ScopedHandle handle_;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  // On Windows, you must not close a HANDLE which is currently being waited on.
  // The MSDN documentation says that the resulting behaviour is 'undefined'.
  // To solve that issue each WaitableEventWatcher duplicates the given event
  // handle.

  // However, if we were to include the following members
  // directly then, on POSIX, one couldn't use WaitableEventWatcher to watch an
  // event which gets deleted. This mismatch has bitten us several times now,
  // so we have a kernel of the WaitableEvent, which is reference counted.
  // WaitableEventWatchers may then take a reference and thus match the Windows
  // behaviour.
  struct WaitableEventKernel :
      public RefCountedThreadSafe<WaitableEventKernel> {
   public:
    WaitableEventKernel(ResetPolicy reset_policy, InitialState initial_state);

    bool Dequeue(Waiter* waiter, void* tag);

    cr::Lock lock_;
    const bool manual_reset_;
    bool signaled_;
    std::list<Waiter*> waiters_;

   private:
    friend class RefCountedThreadSafe<WaitableEventKernel>;
    ~WaitableEventKernel();
  };

  typedef std::pair<WaitableEvent*, size_t> WaiterAndIndex;

  // When dealing with arrays of WaitableEvent*, we want to sort by the address
  // of the WaitableEvent in order to have a globally consistent locking order.
  // In that case we keep them, in sorted order, in an array of pairs where the
  // second element is the index of the WaitableEvent in the original,
  // unsorted, array.
  static size_t EnqueueMany(WaiterAndIndex* waitables,
                            size_t count, Waiter* waiter);

  bool SignalAll();
  bool SignalOne();
  void Enqueue(Waiter* waiter);

  RefPtr<WaitableEventKernel> kernel_;
#endif

  // Whether a thread invoking Wait() on this WaitableEvent should be considered
  // blocked as opposed to idle (and potentially replaced if part of a pool).
  ///bool waiting_is_blocking_ = true;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_WAITABLE_EVENT_H_
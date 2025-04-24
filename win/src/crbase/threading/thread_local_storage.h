// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_LOCAL_STORAGE_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_LOCAL_STORAGE_H_

#include <stdint.h>

#include "crbase/base_export.h"
#include "crbase/atomic/atomicops.h"
#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <pthread.h>
#endif

namespace cr {

namespace internal {

// WARNING: You should *NOT* use this class directly.
// PlatformThreadLocalStorage is a low-level abstraction of the OS's TLS
// interface. Instead, you should use one of the following:
// * ThreadLocalBoolean (from thread_local.h) for booleans.
// * ThreadLocalPointer (from thread_local.h) for pointers.
// * ThreadLocalStorage::StaticSlot/Slot for more direct control of the slot.
class CRBASE_EXPORT PlatformThreadLocalStorage {
 public:

#if defined(MINI_CHROMIUM_OS_WIN)
  typedef unsigned long TLSKey;
  enum : unsigned { TLS_KEY_OUT_OF_INDEXES = TLS_OUT_OF_INDEXES };
#elif defined(MINI_CHROMIUM_OS_POSIX)
  typedef pthread_key_t TLSKey;
  // The following is a "reserved key" which is used in our generic Chromium
  // ThreadLocalStorage implementation.  We expect that an OS will not return
  // such a key, but if it is returned (i.e., the OS tries to allocate it) we
  // will just request another key.
  enum { TLS_KEY_OUT_OF_INDEXES = 0x7FFFFFFF };
#endif

  // The following methods need to be supported on each OS platform, so that
  // the Chromium ThreadLocalStore functionality can be constructed.
  // Chromium will use these methods to acquire a single OS slot, and then use
  // that to support a much larger number of Chromium slots (independent of the
  // OS restrictions).
  // The following returns true if it successfully is able to return an OS
  // key in |key|.
  static bool AllocTLS(TLSKey* key);
  // Note: FreeTLS() doesn't have to be called, it is fine with this leak, OS
  // might not reuse released slot, you might just reset the TLS value with
  // SetTLSValue().
  static void FreeTLS(TLSKey key);
  static void SetTLSValue(TLSKey key, void* value);
  static void* GetTLSValue(TLSKey key);

  // Each platform (OS implementation) is required to call this method on each
  // terminating thread when the thread is about to terminate.  This method
  // will then call all registered destructors for slots in Chromium
  // ThreadLocalStorage, until there are no slot values remaining as having
  // been set on this thread.
  // Destructors may end up being called multiple times on a terminating
  // thread, as other destructors may re-set slots that were previously
  // destroyed.
#if defined(MINI_CHROMIUM_OS_WIN)
  // Since Windows which doesn't support TLS destructor, the implementation
  // should use GetTLSValue() to retrieve the value of TLS slot.
  static void OnThreadExit();
#elif defined(MINI_CHROMIUM_OS_POSIX)
  // |Value| is the data stored in TLS slot, The implementation can't use
  // GetTLSValue() to retrieve the value of slot as it has already been reset
  // in Posix.
  static void OnThreadExit(void* value);
#endif
};

}  // namespace internal

// Wrapper for thread local storage.  This class doesn't do much except provide
// an API for portability.
class CRBASE_EXPORT ThreadLocalStorage {
 public:
  ThreadLocalStorage(const ThreadLocalStorage&) = delete;
  ThreadLocalStorage& operator=(const ThreadLocalStorage&) = delete;

  // Prototype for the TLS destructor function, which can be optionally used to
  // cleanup thread local storage on thread exit.  'value' is the data that is
  // stored in thread local storage.
  typedef void (*TLSDestructorFunc)(void* value);

  // A key representing one value stored in TLS. Use as a class member or a
  // local variable. If you need a static storage duration variable, use the
  // following pattern with a NoDestructor<Slot>:
  // void MyDestructorFunc(void* value);
  // ThreadLocalStorage::Slot& ImportantContentTLS() {
  //   static NoDestructor<ThreadLocalStorage::Slot> important_content_tls(
  //       &MyDestructorFunc);
  //   return *important_content_tls;
  // }
  class CRBASE_EXPORT Slot {
   public:
    Slot(const Slot&) = delete;
    Slot& operator=(const Slot&) = delete;

    explicit Slot(TLSDestructorFunc destructor = NULL);
    ~Slot();

    // Get the thread-local value stored in this slot.
    // Values are guaranteed to initially be zero.
    void* Get() const;

    // Set the slot's thread-local value to |value|.
    void Set(void* value);

   private:
    void Initialize(TLSDestructorFunc destructor);
    void Free();

    static constexpr int kInvalidSlotValue = -1;
    int slot_ = kInvalidSlotValue;
    uint32_t version_ = 0;
  };

 private:
  // In most cases, most callers should not need access to HasBeenDestroyed().
  // If you are working in code that runs during thread destruction, contact the
  // base OWNERs for advice and then make a friend request.
  //
  // Returns |true| if Chrome's implementation of TLS is being or has been
  // destroyed during thread destruction. Attempting to call Slot::Get() during
  // destruction is disallowed and will hit a DCHECK. Any code that relies on
  // TLS during thread destruction must first check this method before calling
  // Slot::Get().
  friend class SequenceCheckerImpl;
  friend class ThreadCheckerImpl;
  static bool HasBeenDestroyed();
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_LOCAL_STORAGE_H_
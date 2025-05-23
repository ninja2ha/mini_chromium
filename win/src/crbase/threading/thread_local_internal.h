// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_LOCAL_INTERNAL_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_LOCAL_INTERNAL_H_

#if CR_DCHECK_IS_ON()

#include <atomic>
#include <memory>

#include "crbase/threading/thread_local_storage.h"

namespace cr {
namespace internal {

// A version of ThreadLocalOwnedPointer which verifies that it's only destroyed
// when no threads, other than the one it is destroyed on, have remaining state
// set in it. A ThreadLocalOwnedPointer instance being destroyed too early would
// result in leaks per unregistering the TLS slot (and thus the DeleteTlsPtr
// hook).
template <typename T>
class CheckedThreadLocalOwnedPointer {
 public:
  CheckedThreadLocalOwnedPointer(
      const CheckedThreadLocalOwnedPointer<T>&) = delete;
  CheckedThreadLocalOwnedPointer<T> operator=(
      const CheckedThreadLocalOwnedPointer<T>&) = delete;
  CheckedThreadLocalOwnedPointer() = default;

  ~CheckedThreadLocalOwnedPointer() {
    Set(nullptr);

    CR_DCHECK(num_assigned_threads_.load(std::memory_order_relaxed) == 0)
        << "Memory leak: Must join all threads or release all associated "
           "thread-local slots before ~ThreadLocalOwnedPointer";
  }

  T* Get() const {
    PtrTracker* const ptr_tracker = static_cast<PtrTracker*>(slot_.Get());
    return ptr_tracker ? ptr_tracker->ptr_.get() : nullptr;
  }

  void Set(std::unique_ptr<T> ptr) {
    delete static_cast<PtrTracker*>(slot_.Get());
    if (ptr)
      slot_.Set(new PtrTracker(this, std::move(ptr)));
    else
      slot_.Set(nullptr);
  }

 private:
  struct PtrTracker {
   public:
    PtrTracker(CheckedThreadLocalOwnedPointer<T>* outer, std::unique_ptr<T> ptr)
        : outer_(outer), ptr_(std::move(ptr)) {
      outer_->num_assigned_threads_.fetch_add(1, std::memory_order_relaxed);
    }

    ~PtrTracker() {
      outer_->num_assigned_threads_.fetch_sub(1, std::memory_order_relaxed);
    }

    CheckedThreadLocalOwnedPointer<T>* const outer_;
    const std::unique_ptr<T> ptr_;
  };

  static void DeleteTlsPtr(void* ptr) { delete static_cast<PtrTracker*>(ptr); }

  ThreadLocalStorage::Slot slot_{&DeleteTlsPtr};

  std::atomic_int num_assigned_threads_{0};
};

}  // namespace internal
}  // namespace cr

#endif  // CR_DCHECK_IS_ON()

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_LOCAL_INTERNAL_H_
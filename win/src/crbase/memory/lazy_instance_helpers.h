// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MEMORY_LAZY_INSTANCE_INTERNAL_H_
#define MINI_CHROMIUM_SRC_CRBASE_MEMORY_LAZY_INSTANCE_INTERNAL_H_

#include "crbase/atomic/atomicops.h"
#include "crbase/base_export.h"
#include "crbase/logging.h"

// Helper methods used by LazyInstance and a few other base APIs for thread-safe
// lazy construction.

namespace cr {
namespace internal {

// Our AtomicWord doubles as a spinlock, where a value of
// kLazyInstanceStateCreating means the spinlock is being held for creation.
constexpr subtle::AtomicWord kLazyInstanceStateCreating = 1;

// Helper for GetOrCreateLazyPointer(). Checks if instance needs to be created.
// If so returns true otherwise if another thread has beat us, waits for
// instance to be created and returns false.
CRBASE_EXPORT bool NeedsLazyInstance(subtle::AtomicWord* state);

// Helper for GetOrCreateLazyPointer(). After creating an instance, this is
// called to register the dtor to be called at program exit and to update the
// atomic state to hold the |new_instance|
CRBASE_EXPORT void CompleteLazyInstance(subtle::AtomicWord* state,
                                        subtle::AtomicWord new_instance,
                                        void (*destructor)(void*),
                                        void* destructor_arg);

}  // namespace internal

namespace subtle {

// If |state| is uninitialized (zero), constructs a value using
// |creator_func(creator_arg)|, stores it into |state| and registers
// |destructor(destructor_arg)| to be called when the current AtExitManager goes
// out of scope. Then, returns the value stored in |state|. It is safe to have
// concurrent calls to this function with the same |state|. |creator_func| may
// return nullptr if it doesn't want to create an instance anymore (e.g. on
// shutdown), it is from then on required to return nullptr to all callers (ref.
// StaticMemorySingletonTraits). In that case, callers need to synchronize
// before |creator_func| may return a non-null instance again (ref.
// StaticMemorySingletonTraits::ResurectForTesting()).
// Implementation note on |creator_func/creator_arg|. It makes for ugly adapters
// but it avoids redundant template instantiations (e.g. saves 27KB in
// chrome.dll) because linker is able to fold these for multiple Types but
// couldn't with the more advanced CreatorFunc template type which in turn
// improves code locality (and application startup) -- ref.
// https://chromium-review.googlesource.com/c/chromium/src/+/530984/5/base/lazy_instance.h#140,
// worsened by https://chromium-review.googlesource.com/c/chromium/src/+/868013
// and caught then as https://crbug.com/804034.
template <typename Type>
Type* GetOrCreateLazyPointer(subtle::AtomicWord* state,
                             Type* (*creator_func)(void*),
                             void* creator_arg,
                             void (*destructor)(void*),
                             void* destructor_arg) {
  CR_DCHECK(state);
  CR_DCHECK(creator_func);

  // If any bit in the created mask is true, the instance has already been
  // fully constructed.
  constexpr subtle::AtomicWord kLazyInstanceCreatedMask =
      ~internal::kLazyInstanceStateCreating;

  // We will hopefully have fast access when the instance is already created.
  // Since a thread sees |state| == 0 or kLazyInstanceStateCreating at most
  // once, the load is taken out of NeedsLazyInstance() as a fast-path. The load
  // has acquire memory ordering as a thread which sees |state| > creating needs
  // to acquire visibility over the associated data. Pairing Release_Store is in
  // CompleteLazyInstance().
  subtle::AtomicWord instance = subtle::Acquire_Load(state);
  if (!(instance & kLazyInstanceCreatedMask)) {
    if (internal::NeedsLazyInstance(state)) {
      // This thread won the race and is now responsible for creating the
      // instance and storing it back into |state|.
      instance =
          reinterpret_cast<subtle::AtomicWord>((*creator_func)(creator_arg));
      internal::CompleteLazyInstance(state, instance, destructor,
                                     destructor_arg);
    } else {
      // This thread lost the race but now has visibility over the constructed
      // instance (NeedsLazyInstance() doesn't return until the constructing
      // thread releases the instance via CompleteLazyInstance()).
      instance = subtle::Acquire_Load(state);
      CR_DCHECK(instance & kLazyInstanceCreatedMask);
    }
  }
  return reinterpret_cast<Type*>(instance);
}

}  // namespace subtle

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MEMORY_LAZY_INSTANCE_INTERNAL_H_
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This defines helpful methods for dealing with Callbacks.  Because Callbacks
// are implemented using templates, with a class per callback signature, adding
// methods to Callback<> itself is unattractive (lots of extra code gets
// generated).  Instead, consider adding methods here.
//
// ResetAndReturn(&cb) is like cb.Reset() but allows executing a callback (via a
// move or copy) after the original callback is Reset().  This can be handy if
// Run() reads/writes the variable holding the Callback.

#ifndef MINI_CHROMIUM_CRBASE_FUNCTIONAL_CALLBACK_HELPERS_H_
#define MINI_CHROMIUM_CRBASE_FUNCTIONAL_CALLBACK_HELPERS_H_

#include <utility>
#include <memory>

#include "crbase/atomic/atomicops.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/callback.h"
#include "crbase/compiler_specific.h"

namespace cr {

template <typename Signature,
          internal::CopyMode copy_mode,
          internal::RepeatMode repeat_mode>
Callback<Signature, copy_mode, repeat_mode> ResetAndReturn(
    Callback<Signature, copy_mode, repeat_mode>* cb) {
  Callback<Signature, copy_mode, repeat_mode> ret(std::move(*cb));
  CR_DCHECK(!*cb);
  return ret;
}

namespace internal {

template <typename... Args>
class AdaptCallbackForRepeatingHelper final {
 public:
  AdaptCallbackForRepeatingHelper(
      const AdaptCallbackForRepeatingHelper&) = delete;
  AdaptCallbackForRepeatingHelper& operator=(
      const AdaptCallbackForRepeatingHelper&) = delete;

  explicit AdaptCallbackForRepeatingHelper(OnceCallback<void(Args...)> callback)
      : callback_(std::move(callback)) {
    CR_DCHECK(callback_);
  }

  void Run(Args... args) {
    if (subtle::NoBarrier_AtomicExchange(&has_run_, 1))
      return;
    CR_DCHECK(callback_);
    std::move(callback_).Run(std::forward<Args>(args)...);
  }

 private:
  volatile subtle::Atomic32 has_run_ = 0;
  cr::OnceCallback<void(Args...)> callback_;
};

}  // namespace internal

// Wraps the given OnceCallback into a RepeatingCallback that relays its
// invocation to the original OnceCallback on the first invocation. The
// following invocations are just ignored.
template <typename... Args>
RepeatingCallback<void(Args...)> AdaptCallbackForRepeating(
    OnceCallback<void(Args...)> callback) {
  using Helper = internal::AdaptCallbackForRepeatingHelper<Args...>;
  return cr::BindRepeating(&Helper::Run,
                           std::make_unique<Helper>(std::move(callback)));
}

// ScopedClosureRunner is akin to std::unique_ptr<> for Closures. It ensures
// that the Closure is executed no matter how the current scope exits.
class CRBASE_EXPORT ScopedClosureRunner {
 public:
  ScopedClosureRunner(const ScopedClosureRunner&) = delete;
  ScopedClosureRunner& operator=(const ScopedClosureRunner&) = delete;

  ScopedClosureRunner();
  explicit ScopedClosureRunner(OnceClosure closure);
  ~ScopedClosureRunner();

  ScopedClosureRunner(ScopedClosureRunner&& other);

  // Releases the current closure if it's set and replaces it with the closure
  // from |other|.
  ScopedClosureRunner& operator=(ScopedClosureRunner&& other);

  // Calls the current closure and resets it, so it wont be called again.
  void RunAndReset();

  // Replaces closure with the new one releasing the old one without calling it.
  void ReplaceClosure(OnceClosure closure);

  // Releases the Closure without calling.
  OnceClosure Release() CR_WARN_UNUSED_RESULT;

 private:
  OnceClosure closure_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_CRBASE_FUNCTIONAL_CALLBACK_HELPERS_H_
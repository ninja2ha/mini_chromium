// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains utility functions and classes that help the
// implementation, and management of the Callback objects.

#ifndef MINI_CHROMIUM_CRBASE_FUNCTIONAL_CALLBACK_INTERNAL_H_
#define MINI_CHROMIUM_CRBASE_FUNCTIONAL_CALLBACK_INTERNAL_H_

#include "crbase/base_export.h"
#include "crbase/functional/callback_forward.h"
#include "crbase/memory/ref_counted.h"

namespace cr {

struct FakeBindState;

namespace internal {

template <CopyMode copy_mode>
class CallbackBase;

class BindStateBase;

template <typename Functor, typename... BoundArgs>
struct BindState;

struct BindStateBaseRefCountTraits {
  static void Destruct(const BindStateBase*);
};

// BindStateBase is used to provide an opaque handle that the Callback
// class can use to represent a function object with bound arguments.  It
// behaves as an existential type that is used by a corresponding
// DoInvoke function to perform the function execution.  This allows
// us to shield the Callback class from the types of the bound argument via
// "type erasure."
// At the base level, the only task is to add reference counting data. Don't use
// RefCountedThreadSafe since it requires the destructor to be a virtual method.
// Creating a vtable for every BindState template instantiation results in a lot
// of bloat. Its only task is to call the destructor which can be done with a
// function pointer.
class CRBASE_EXPORT BindStateBase
    : public RefCountedThreadSafe<BindStateBase, BindStateBaseRefCountTraits> {
 public:
  CR_REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE;

  using InvokeFuncStorage = void(*)();

  BindStateBase(const BindStateBase&) = delete;
  BindStateBase& operator=(const BindStateBase&) = delete;

 private:
  BindStateBase(InvokeFuncStorage polymorphic_invoke,
                void (*destructor)(const BindStateBase*));
  BindStateBase(InvokeFuncStorage polymorphic_invoke,
                void (*destructor)(const BindStateBase*),
                bool (*is_cancelled)(const BindStateBase*));

  ~BindStateBase() = default;

  friend struct BindStateBaseRefCountTraits;
  friend class RefCountedThreadSafe<BindStateBase, BindStateBaseRefCountTraits>;

  template <CopyMode copy_mode>
  friend class CallbackBase;

  // Whitelist subclasses that access the destructor of BindStateBase.
  template <typename Functor, typename... BoundArgs>
  friend struct BindState;
  friend struct ::cr::FakeBindState;

  bool IsCancelled() const {
    return is_cancelled_(this);
  }

  // In C++, it is safe to cast function pointers to function pointers of
  // another type. It is not okay to use void*. We create a InvokeFuncStorage
  // that that can store our function pointer, and then cast it back to
  // the original type on usage.
  InvokeFuncStorage polymorphic_invoke_;

  // Pointer to a function that will properly destroy |this|.
  void (*destructor_)(const BindStateBase*);
  bool (*is_cancelled_)(const BindStateBase*);
};

// Holds the Callback methods that don't require specialization to reduce
// template bloat.
// CallbackBase<MoveOnly> is a direct base class of MoveOnly callbacks, and
// CallbackBase<Copyable> uses CallbackBase<MoveOnly> for its implementation.
template <>
class CRBASE_EXPORT CallbackBase<CopyMode::MoveOnly> {
 public:
  CallbackBase(CallbackBase&& c);
  CallbackBase& operator=(CallbackBase&& c);

  explicit CallbackBase(const CallbackBase<CopyMode::Copyable>& c);
  CallbackBase& operator=(const CallbackBase<CopyMode::Copyable>& c);

  explicit CallbackBase(CallbackBase<CopyMode::Copyable>&& c);
  CallbackBase& operator=(CallbackBase<CopyMode::Copyable>&& c);

  // Returns true if Callback is null (doesn't refer to anything).
  bool is_null() const { return !bind_state_; }
  explicit operator bool() const { return !is_null(); }

  // Returns true if the callback invocation will be nop due to an cancellation.
  // It's invalid to call this on uninitialized callback.
  bool IsCancelled() const;

  // Returns the Callback into an uninitialized state.
  void Reset();

 protected:
  using InvokeFuncStorage = BindStateBase::InvokeFuncStorage;

  // Returns true if this callback equals |other|. |other| may be null.
  bool EqualsInternal(const CallbackBase& other) const;

  // Allow initializing of |bind_state_| via the constructor to avoid default
  // initialization of the scoped_refptr.
  explicit CallbackBase(BindStateBase* bind_state);

  InvokeFuncStorage polymorphic_invoke() const {
    return bind_state_->polymorphic_invoke_;
  }

  // Force the destructor to be instantiated inside this translation unit so
  // that our subclasses will not get inlined versions.  Avoids more template
  // bloat.
  ~CallbackBase();

  RefPtr<BindStateBase> bind_state_;
};

// CallbackBase<Copyable> is a direct base class of Copyable Callbacks.
template <>
class CRBASE_EXPORT CallbackBase<CopyMode::Copyable>
    : public CallbackBase<CopyMode::MoveOnly> {
 public:
  CallbackBase(const CallbackBase& c);
  CallbackBase(CallbackBase&& c);
  CallbackBase& operator=(const CallbackBase& c);
  CallbackBase& operator=(CallbackBase&& c);
 protected:
  explicit CallbackBase(BindStateBase* bind_state)
      : CallbackBase<CopyMode::MoveOnly>(bind_state) {}
  ~CallbackBase() {}
};

extern template class CallbackBase<CopyMode::MoveOnly>;
extern template class CallbackBase<CopyMode::Copyable>;

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_CRBASE_FUNCTIONAL_CALLBACK_INTERNAL_H_
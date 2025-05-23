// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_FUNCTIONAL_BIND_INTERNAL_H_
#define MINI_CHROMIUM_CRBASE_FUNCTIONAL_BIND_INTERNAL_H_

#include <stddef.h>

#include <tuple>
#include <type_traits>

#include "crbase/memory/weak_ptr.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/containers/tuple.h"
#include "crbase/helper/template_util.h"
#include "crbase/functional/bind_helpers.h"
#include "crbase/functional/callback_internal.h"
#include "crbase/build_platform.h"

namespace cr {
namespace internal {

// See crbase/functional/callback.h for user documentation.
//
//
// CONCEPTS:
//  Functor -- A movable type representing something that should be called.
//             All function pointers and Callback<> are functors even if the
//             invocation syntax differs.
//  RunType -- A function type (as opposed to function _pointer_ type) for
//             a Callback<>::Run().  Usually just a convenience typedef.
//  (Bound)Args -- A set of types that stores the arguments.
//
// Types:
//  ForceVoidReturn<> -- Helper class for translating function signatures to
//                       equivalent forms with a "void" return type.
//  FunctorTraits<> -- Type traits used to determine the correct RunType and
//                     invocation manner for a Functor.  This is where function
//                     signature adapters are applied.
//  InvokeHelper<> -- Take a Functor + arguments and actully invokes it.
//                    Handle the differing syntaxes needed for WeakPtr<>
//                    support.  This is separate from Invoker to avoid creating
//                    multiple version of Invoker<>.
//  Invoker<> -- Unwraps the curried parameters and executes the Functor.
//  BindState<> -- Stores the curried parameters, and is the main entry point
//                 into the Bind() system.

template <typename T>
struct NeedsScopedRefptrButGetsRawPtr {
  enum {
    // Human readable translation: you needed to be a scoped_refptr if you are a
    // raw pointer type and are convertible to a RefCounted(Base|ThreadSafeBase)
    // type.
    value = (std::is_pointer<T>::value &&
             (std::is_convertible<T, subtle::RefCountedBase*>::value ||
              std::is_convertible<T, subtle::RefCountedThreadSafeBase*>::value))
  };
};

template <typename Params>
struct ParamsUseScopedRefptrCorrectly {
  enum { value = 0 };
};

template <>
struct ParamsUseScopedRefptrCorrectly<std::tuple<>> {
  enum { value = 1 };
};

template <typename Head, typename... Tail>
struct ParamsUseScopedRefptrCorrectly<std::tuple<Head, Tail...>> {
  enum { value = !NeedsScopedRefptrButGetsRawPtr<Head>::value &&
                  ParamsUseScopedRefptrCorrectly<std::tuple<Tail...>>::value };
};

template <typename Callable,
          typename Signature = decltype(&Callable::operator())>
struct ExtractCallableRunTypeImpl;

template <typename Callable, typename R, typename... Args>
struct ExtractCallableRunTypeImpl<Callable, R(Callable::*)(Args...) const> {
  using Type = R(Args...);
};

// Evaluated to RunType of the given callable type.
// Example:
//   auto f = [](int, char*) { return 0.1; };
//   ExtractCallableRunType<decltype(f)>
//   is evaluated to
//   double(int, char*);
template <typename Callable>
using ExtractCallableRunType =
    typename ExtractCallableRunTypeImpl<Callable>::Type;

// IsConvertibleToRunType<Functor> is std::true_type if |Functor| has operator()
// and convertible to the corresponding function pointer. Otherwise, it's
// std::false_type.
// Example:
//   IsConvertibleToRunType<void(*)()>::value is false.
//
//   struct Foo {};
//   IsConvertibleToRunType<void(Foo::*)()>::value is false.
//
//   auto f = []() {};
//   IsConvertibleToRunType<decltype(f)>::value is true.
//
//   int i = 0;
//   auto g = [i]() {};
//   IsConvertibleToRunType<decltype(g)>::value is false.
template <typename Functor, typename SFINAE = void>
struct IsConvertibleToRunType : std::false_type {};

template <typename Callable>
struct IsConvertibleToRunType<Callable, void_t<decltype(&Callable::operator())>>
    : std::is_convertible<Callable, ExtractCallableRunType<Callable>*> {};

// HasRefCountedTypeAsRawPtr selects true_type when any of the |Args| is a raw
// pointer to a RefCounted type.
// Implementation note: This non-specialized case handles zero-arity case only.
// Non-zero-arity cases should be handled by the specialization below.
template <typename... Args>
struct HasRefCountedTypeAsRawPtr : std::false_type {};

// Implementation note: Select true_type if the first parameter is a raw pointer
// to a RefCounted type. Otherwise, skip the first parameter and check rest of
// parameters recursively.
template <typename T, typename... Args>
struct HasRefCountedTypeAsRawPtr<T, Args...>
    : std::conditional<NeedsScopedRefptrButGetsRawPtr<T>::value,
                       std::true_type,
                       HasRefCountedTypeAsRawPtr<Args...>>::type {};

// ForceVoidReturn<>
//
// Set of templates that support forcing the function return type to void.
template <typename Sig>
struct ForceVoidReturn;

template <typename R, typename... Args>
struct ForceVoidReturn<R(Args...)> {
  using RunType = void(Args...);
};

// FunctorTraits<>
//
// See description at top of file.
template <typename Functor, typename SFINAE>
struct FunctorTraits;

// For a callable type that is convertible to the corresponding function type.
// This specialization is intended to allow binding captureless lambdas by
// base::Bind(), based on the fact that captureless lambdas can be convertible
// to the function type while capturing lambdas can't.
template <typename Functor>
struct FunctorTraits<
    Functor,
    typename std::enable_if<IsConvertibleToRunType<Functor>::value>::type> {
  using RunType = ExtractCallableRunType<Functor>;
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = false;

  template <typename... RunArgs>
  static ExtractReturnType<RunType>
  Invoke(const Functor& functor, RunArgs&&... args) {
    return functor(std::forward<RunArgs>(args)...);
  }
};

// For functions.
template <typename R, typename... Args>
struct FunctorTraits<R (*)(Args...)> {
  using RunType = R(Args...);
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = true;

  template <typename... RunArgs>
  static R Invoke(R (*function)(Args...), RunArgs&&... args) {
    return function(std::forward<RunArgs>(args)...);
  }
};

#if defined(MINI_CHROMIUM_OS_WIN) && !defined(MINI_CHROMIUM_ARCH_CPU_X86_64)

// For functions.
template <typename R, typename... Args>
struct FunctorTraits<R(__stdcall*)(Args...)> {
  using RunType = R(Args...);
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = true;

  template <typename... RunArgs>
  static R Invoke(R(__stdcall* function)(Args...), RunArgs&&... args) {
    return function(std::forward<RunArgs>(args)...);
  }
};

// For functions.
template <typename R, typename... Args>
struct FunctorTraits<R(__fastcall*)(Args...)> {
  using RunType = R(Args...);
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = true;

  template <typename... RunArgs>
  static R Invoke(R(__fastcall* function)(Args...), RunArgs&&... args) {
    return function(std::forward<RunArgs>(args)...);
  }
};

#endif  // defined(MINI_CHROMIUM_OS_WIN) && 
        // !defined(MINI_CHROMIUM_ARCH_CPU_X86_64)

// For methods.
template <typename R, typename Receiver, typename... Args>
struct FunctorTraits<R (Receiver::*)(Args...)> {
  using RunType = R(Receiver*, Args...);
  static constexpr bool is_method = true;
  static constexpr bool is_nullable = true;

  template <typename ReceiverPtr, typename... RunArgs>
  static R Invoke(R (Receiver::*method)(Args...),
                  ReceiverPtr&& receiver_ptr,
                  RunArgs&&... args) {
    // Clang skips CV qualifier check on a method pointer invocation when the
    // receiver is a subclass. Store the receiver into a const reference to
    // T to ensure the CV check works.
    // https://llvm.org/bugs/show_bug.cgi?id=27037
    Receiver& receiver = *receiver_ptr;
    return (receiver.*method)(std::forward<RunArgs>(args)...);
  }
};

// For const methods.
template <typename R, typename Receiver, typename... Args>
struct FunctorTraits<R (Receiver::*)(Args...) const> {
  using RunType = R(const Receiver*, Args...);
  static constexpr bool is_method = true;
  static constexpr bool is_nullable = true;

  template <typename ReceiverPtr, typename... RunArgs>
  static R Invoke(R (Receiver::*method)(Args...) const,
                  ReceiverPtr&& receiver_ptr,
                  RunArgs&&... args) {
    // Clang skips CV qualifier check on a method pointer invocation when the
    // receiver is a subclass. Store the receiver into a const reference to
    // T to ensure the CV check works.
    // https://llvm.org/bugs/show_bug.cgi?id=27037
    const Receiver& receiver = *receiver_ptr;
    return (receiver.*method)(std::forward<RunArgs>(args)...);
  }
};

// For IgnoreResults.
template <typename T>
struct FunctorTraits<IgnoreResultHelper<T>> : FunctorTraits<T> {
  using RunType =
      typename ForceVoidReturn<typename FunctorTraits<T>::RunType>::RunType;

  template <typename IgnoreResultType, typename... RunArgs>
  static void Invoke(IgnoreResultType&& ignore_result_helper,
                     RunArgs&&... args) {
    FunctorTraits<T>::Invoke(
        std::forward<IgnoreResultType>(ignore_result_helper).functor_,
        std::forward<RunArgs>(args)...);
  }
};

// For Callbacks.
template <typename R, typename... Args,
          CopyMode copy_mode, RepeatMode repeat_mode>
struct FunctorTraits<Callback<R(Args...), copy_mode, repeat_mode>> {
  using RunType = R(Args...);
  static constexpr bool is_method = false;
  static constexpr bool is_nullable = true;

  template <typename CallbackType, typename... RunArgs>
  static R Invoke(CallbackType&& callback, RunArgs&&... args) {
    CR_DCHECK(!callback.is_null());
    return std::forward<CallbackType>(callback).Run(
        std::forward<RunArgs>(args)...);
  }
};

// InvokeHelper<>
//
// There are 2 logical InvokeHelper<> specializations: normal, WeakCalls.
//
// The normal type just calls the underlying runnable.
//
// WeakCalls need special syntax that is applied to the first argument to check
// if they should no-op themselves.
template <bool is_weak_call, typename ReturnType>
struct InvokeHelper;

template <typename ReturnType>
struct InvokeHelper<false, ReturnType> {
  template <typename Functor, typename... RunArgs>
  static inline ReturnType MakeItSo(Functor&& functor, RunArgs&&... args) {
    using Traits = FunctorTraits<typename std::decay<Functor>::type>;
    return Traits::Invoke(std::forward<Functor>(functor),
                          std::forward<RunArgs>(args)...);
  }
};

template <typename ReturnType>
struct InvokeHelper<true, ReturnType> {
  // WeakCalls are only supported for functions with a void return type.
  // Otherwise, the function result would be undefined if the the WeakPtr<>
  // is invalidated.
  static_assert(std::is_void<ReturnType>::value,
                "weak_ptrs can only bind to methods without return values");

  template <typename Functor, typename BoundWeakPtr, typename... RunArgs>
  static inline void MakeItSo(Functor&& functor,
                              BoundWeakPtr&& weak_ptr,
                              RunArgs&&... args) {
    if (!weak_ptr)
      return;
    using Traits = FunctorTraits<typename std::decay<Functor>::type>;
    Traits::Invoke(std::forward<Functor>(functor),
                   std::forward<BoundWeakPtr>(weak_ptr),
                   std::forward<RunArgs>(args)...);
  }
};

// Invoker<>
//
// See description at the top of the file.
template <typename StorageType, typename UnboundRunType>
struct Invoker;

template <typename StorageType, typename R, typename... UnboundArgs>
struct Invoker<StorageType, R(UnboundArgs...)> {
  static R RunOnce(BindStateBase* base, UnboundArgs&&... unbound_args) {
    // Local references to make debugger stepping easier. If in a debugger,
    // you really want to warp ahead and step through the
    // InvokeHelper<>::MakeItSo() call below.
    StorageType* storage = static_cast<StorageType*>(base);
    static constexpr size_t num_bound_args =
        std::tuple_size<decltype(storage->bound_args_)>::value;
    return RunImpl(std::move(storage->functor_),
                   std::move(storage->bound_args_),
                   MakeIndexSequence<num_bound_args>(),
                   std::forward<UnboundArgs>(unbound_args)...);
  }

  static R Run(BindStateBase* base, UnboundArgs&&... unbound_args) {
    // Local references to make debugger stepping easier. If in a debugger,
    // you really want to warp ahead and step through the
    // InvokeHelper<>::MakeItSo() call below.
    const StorageType* storage = static_cast<StorageType*>(base);
    static constexpr size_t num_bound_args =
        std::tuple_size<decltype(storage->bound_args_)>::value;
    return RunImpl(storage->functor_,
                   storage->bound_args_,
                   MakeIndexSequence<num_bound_args>(),
                   std::forward<UnboundArgs>(unbound_args)...);
  }

 private:
  template <typename Functor, typename BoundArgsTuple, size_t... indices>
  static inline R RunImpl(Functor&& functor,
                          BoundArgsTuple&& bound,
                          IndexSequence<indices...>,
                          UnboundArgs&&... unbound_args) {
    static constexpr bool is_method =
        FunctorTraits<typename std::decay<Functor>::type>::is_method;

    using DecayedArgsTuple = typename std::decay<BoundArgsTuple>::type;
    static constexpr bool is_weak_call =
        IsWeakMethod<is_method,
                     typename std::tuple_element<
                         indices,
                         DecayedArgsTuple>::type...>::value;

    return InvokeHelper<is_weak_call, R>::MakeItSo(
        std::forward<Functor>(functor),
        Unwrap(std::get<indices>(std::forward<BoundArgsTuple>(bound)))...,
        std::forward<UnboundArgs>(unbound_args)...);
  }
};

// Used to implement MakeUnboundRunType.
template <typename Functor, typename... BoundArgs>
struct MakeUnboundRunTypeImpl {
  using RunType =
      typename FunctorTraits<typename std::decay<Functor>::type>::RunType;
  using ReturnType = ExtractReturnType<RunType>;
  using Args = ExtractArgs<RunType>;
  using UnboundArgs = DropTypeListItem<sizeof...(BoundArgs), Args>;
  using Type = MakeFunctionType<ReturnType, UnboundArgs>;
};
template <typename Functor>
typename std::enable_if<FunctorTraits<Functor>::is_nullable, bool>::type
IsNull(const Functor& functor) {
  return !functor;
}

template <typename Functor>
typename std::enable_if<!FunctorTraits<Functor>::is_nullable, bool>::type
IsNull(const Functor&) {
  return false;
}

// Used by ApplyCancellationTraits below.
template <typename Functor, typename BoundArgsTuple, size_t... indices>
bool ApplyCancellationTraitsImpl(const Functor& functor,
                                 const BoundArgsTuple& bound_args,
                                 IndexSequence<indices...>) {
  return CallbackCancellationTraits<Functor, BoundArgsTuple>::IsCancelled(
      functor, std::get<indices>(bound_args)...);
}

// Relays |base| to corresponding CallbackCancellationTraits<>::Run(). Returns
// true if the callback |base| represents is canceled.
template <typename BindStateType>
bool ApplyCancellationTraits(const BindStateBase* base) {
  const BindStateType* storage = static_cast<const BindStateType*>(base);
  static constexpr size_t num_bound_args =
      std::tuple_size<decltype(storage->bound_args_)>::value;
  return ApplyCancellationTraitsImpl(storage->functor_, storage->bound_args_,
                                     MakeIndexSequence<num_bound_args>());
};

// Template helpers to detect using Bind() on a base::Callback without any
// additional arguments. In that case, the original base::Callback object should
// just be directly used.
template <typename Functor, typename... BoundArgs>
struct BindingCallbackWithNoArgs {
  static constexpr bool value = false;
};

template <typename Signature,
          typename... BoundArgs,
          CopyMode copy_mode,
          RepeatMode repeat_mode>
struct BindingCallbackWithNoArgs<Callback<Signature, copy_mode, repeat_mode>,
                                 BoundArgs...> {
  static constexpr bool value = sizeof...(BoundArgs) == 0;
};

// BindState<>
//
// This stores all the state passed into Bind().
template <typename Functor, typename... BoundArgs>
struct BindState final : BindStateBase {
  using IsCancellable = std::integral_constant<
      bool,
      CallbackCancellationTraits<Functor,
                                 std::tuple<BoundArgs...>>::is_cancellable>;

  template <typename ForwardFunctor, typename... ForwardBoundArgs>
  explicit BindState(BindStateBase::InvokeFuncStorage invoke_func,
                     ForwardFunctor&& functor,
                     ForwardBoundArgs&&... bound_args)
      // IsCancellable is std::false_type if
      // CallbackCancellationTraits<>::IsCancelled returns always false.
      // Otherwise, it's std::true_type.
      : BindState(IsCancellable{},
                  invoke_func,
                  std::forward<ForwardFunctor>(functor),
                  std::forward<ForwardBoundArgs>(bound_args)...) {
    static_assert(!BindingCallbackWithNoArgs<Functor, BoundArgs...>::value,
                  "Attempting to bind a base::Callback with no additional "
                  "arguments: save a heap allocation and use the original "
                  "cr::Callback object");
  }

  Functor functor_;
  std::tuple<BoundArgs...> bound_args_;

 private:
  template <typename ForwardFunctor, typename... ForwardBoundArgs>
  explicit BindState(std::true_type,
                     BindStateBase::InvokeFuncStorage invoke_func,
                     ForwardFunctor&& functor,
                     ForwardBoundArgs&&... bound_args)
      : BindStateBase(invoke_func,
                      &Destroy,
                      &ApplyCancellationTraits<BindState>),
        functor_(std::forward<ForwardFunctor>(functor)),
        bound_args_(std::forward<ForwardBoundArgs>(bound_args)...) {
    CR_DCHECK(!IsNull(functor_));
  }

  template <typename ForwardFunctor, typename... ForwardBoundArgs>
  explicit BindState(std::false_type,
                     BindStateBase::InvokeFuncStorage invoke_func,
                     ForwardFunctor&& functor,
                     ForwardBoundArgs&&... bound_args)
      : BindStateBase(invoke_func, &Destroy),
        functor_(std::forward<ForwardFunctor>(functor)),
        bound_args_(std::forward<ForwardBoundArgs>(bound_args)...) {
    CR_DCHECK(!IsNull(functor_));
  }

  ~BindState() {}

  static void Destroy(const BindStateBase* self) {
    delete static_cast<const BindState*>(self);
  }
};

// Used to implement MakeBindStateType.
template <bool is_method, typename Functor, typename... BoundArgs>
struct MakeBindStateTypeImpl;

template <typename Functor, typename... BoundArgs>
struct MakeBindStateTypeImpl<false, Functor, BoundArgs...> {
  static_assert(!HasRefCountedTypeAsRawPtr<BoundArgs...>::value,
                "A parameter is a refcounted type and needs scoped_refptr.");
  using Type = BindState<typename std::decay<Functor>::type,
                         typename std::decay<BoundArgs>::type...>;
};

template <typename Functor>
struct MakeBindStateTypeImpl<true, Functor> {
  using Type = BindState<typename std::decay<Functor>::type>;
};

template <typename Functor, typename Receiver, typename... BoundArgs>
struct MakeBindStateTypeImpl<true, Functor, Receiver, BoundArgs...> {
  static_assert(
      !std::is_array<typename std::remove_reference<Receiver>::type>::value,
      "First bound argument to a method cannot be an array.");
  static_assert(!HasRefCountedTypeAsRawPtr<BoundArgs...>::value,
                "A parameter is a refcounted type and needs scoped_refptr.");

 private:
  using DecayedReceiver = typename std::decay<Receiver>::type;

 public:
  using Type = BindState<
      typename std::decay<Functor>::type,
      typename std::conditional<
          std::is_pointer<DecayedReceiver>::value,
          RefPtr<typename std::remove_pointer<DecayedReceiver>::type>,
          DecayedReceiver>::type,
      typename std::decay<BoundArgs>::type...>;
};

template <typename Functor, typename... BoundArgs>
using MakeBindStateType = typename MakeBindStateTypeImpl<
    FunctorTraits<typename std::decay<Functor>::type>::is_method,
    Functor,
    BoundArgs...>::Type;

}  // namespace internal

// Returns a RunType of bound functor.
// E.g. MakeUnboundRunType<R(A, B, C), A, B> is evaluated to R(C).
template <typename Functor, typename... BoundArgs>
using MakeUnboundRunType =
    typename internal::MakeUnboundRunTypeImpl<Functor, BoundArgs...>::Type;

}  // namespace cr

#endif  // MINI_CHROMIUM_CRBASE_FUNCTIONAL_BIND_INTERNAL_H_
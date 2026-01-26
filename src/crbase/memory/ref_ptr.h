// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_PTR_H_
#define MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_PTR_H_

#include <stddef.h>

#include <iosfwd>
#include <type_traits>
#include <utility>

#include "crbase/logging/logging.h"
#include "crbuild/compiler_specific.h"

namespace cr {

template <class T>
class RefPtr;

template <class, typename>
class RefCounted;
template <class, typename>
class RefCountedThreadSafe;
class SequencedTaskRunner;
class WrappedPromise;

template <typename T>
RefPtr<T> AdoptRef(T* t);

namespace internal {

class BasePromise;

}  // namespace internal

namespace subtle {

enum AdoptRefTag { kAdoptRefTag };
enum StartRefCountFromZeroTag { kStartRefCountFromZeroTag };
enum StartRefCountFromOneTag { kStartRefCountFromOneTag };

template <typename T, typename U, typename V>
constexpr bool IsRefCountPreferenceOverridden(const T*,
                                              const RefCounted<U, V>*) {
  return !std::is_same<std::decay_t<decltype(T::kRefCountPreference)>,
                       std::decay_t<decltype(U::kRefCountPreference)>>::value;
}

template <typename T, typename U, typename V>
constexpr bool IsRefCountPreferenceOverridden(
    const T*,
    const RefCountedThreadSafe<U, V>*) {
  return !std::is_same<std::decay_t<decltype(T::kRefCountPreference)>,
                       std::decay_t<decltype(U::kRefCountPreference)>>::value;
}

constexpr bool IsRefCountPreferenceOverridden(...) {
  return false;
}

}  // namespace subtle

// Creates a scoped_refptr from a raw pointer without incrementing the reference
// count. Use this only for a newly created object whose reference count starts
// from 1 instead of 0.
template <typename T>
RefPtr<T> AdoptRef(T* obj) {
  using Tag = std::decay_t<decltype(T::kRefCountPreference)>;
  static_assert(std::is_same<subtle::StartRefCountFromOneTag, Tag>::value,
                "Use AdoptRef only if the reference count starts from one.");

  CR_DCHECK(obj);
  CR_DCHECK(obj->HasOneRef());
  obj->Adopted();
  return RefPtr<T>(obj, subtle::kAdoptRefTag);
}

namespace subtle {

template <typename T>
RefPtr<T> AdoptRefIfNeeded(T* obj, StartRefCountFromZeroTag) {
  return RefPtr<T>(obj);
}

template <typename T>
RefPtr<T> AdoptRefIfNeeded(T* obj, StartRefCountFromOneTag) {
  return AdoptRef(obj);
}

}  // namespace subtle

// Constructs an instance of T, which is a ref counted type, and wraps the
// object into a scoped_refptr<T>.
template <typename T, typename... Args>
RefPtr<T> MakeRefCounted(Args&&... args) {
  T* obj = new T(std::forward<Args>(args)...);
  return subtle::AdoptRefIfNeeded(obj, T::kRefCountPreference);
}

// Takes an instance of T, which is a ref counted type, and wraps the object
// into a scoped_refptr<T>.
template <typename T>
RefPtr<T> WrapRefCounted(T* t) {
  return RefPtr<T>(t);
}

//
// A smart pointer class for reference counted objects.  Use this class instead
// of calling AddRef and Release manually on a reference counted object to
// avoid common memory leaks caused by forgetting to Release an object
// reference.  Sample usage:
//
//   class MyFoo : public RefCounted<MyFoo> {
//    ...
//    private:
//     friend class RefCounted<MyFoo>;  // Allow destruction by RefCounted<>.
//     ~MyFoo();                        // Destructor must be private/protected.
//   };
//
//   void some_function() {
//     RefPtr<MyFoo> foo = MakeRefCounted<MyFoo>();
//     foo->Method(param);
//     // |foo| is released when this function returns
//   }
//
//   void some_other_function() {
//     RefPtr<MyFoo> foo = MakeRefCounted<MyFoo>();
//     ...
//     foo.reset();  // explicitly releases |foo|
//     ...
//     if (foo)
//       foo->Method(param);
//   }
//
// The above examples show how scoped_refptr<T> acts like a pointer to T.
// Given two scoped_refptr<T> classes, it is also possible to exchange
// references between the two objects, like so:
//
//   {
//     RefPtr<MyFoo> a = MakeRefCounted<MyFoo>();
//     RefPtr<MyFoo> b;
//
//     b.swap(a);
//     // now, |b| references the MyFoo object, and |a| references nullptr.
//   }
//
// To make both |a| and |b| in the above example reference the same MyFoo
// object, simply use the assignment operator:
//
//   {
//     RefPtr<MyFoo> a = MakeRefCounted<MyFoo>();
//     RefPtr<MyFoo> b;
//
//     b = a;
//     // now, |a| and |b| each own a reference to the same MyFoo object.
//   }
//
// Also see Chromium's ownership and calling conventions:
// https://chromium.googlesource.com/chromium/src/+/lkgr/styleguide/c++/c++.md#object-ownership-and-calling-conventions
// Specifically:
//   If the function (at least sometimes) takes a ref on a refcounted object,
//   declare the param as RefPtr<T>. The caller can decide whether it
//   wishes to transfer ownership (by calling std::move(t) when passing t) or
//   retain its ref (by simply passing t directly).
//   In other words, use scoped_refptr like you would a std::unique_ptr except
//   in the odd case where it's required to hold on to a ref while handing one
//   to another component (if a component merely needs to use t on the stack
//   without keeping a ref: pass t as a raw T*).
template <class T>
class RefPtr {
 public:
  typedef T element_type;

  constexpr RefPtr() = default;

  // Allow implicit construction from nullptr.
  constexpr RefPtr(std::nullptr_t) {}

  // Constructs from a raw pointer. Note that this constructor allows implicit
  // conversion from T* to scoped_refptr<T> which is strongly discouraged. If
  // you are creating a new ref-counted object please use
  // cr::MakeRefCounted<T>() or cr::WrapRefCounted<T>(). Otherwise you
  // should move or copy construct from an existing scoped_refptr<T> to the
  // ref-counted object.
  RefPtr(T* p) : ptr_(p) {
    if (ptr_)
      AddRef(ptr_);
  }

  // Copy constructor. This is required in addition to the copy conversion
  // constructor below.
  RefPtr(const RefPtr& r) : RefPtr(r.ptr_) {}

  // Copy conversion constructor.
  template <typename U,
            typename = typename std::enable_if<
                std::is_convertible<U*, T*>::value>::type>
  RefPtr(const RefPtr<U>& r) : RefPtr(r.ptr_) {}

  // Move constructor. This is required in addition to the move conversion
  // constructor below.
  RefPtr(RefPtr&& r) noexcept : ptr_(r.ptr_) { r.ptr_ = nullptr; }

  // Move conversion constructor.
  template <typename U,
            typename = typename std::enable_if<
                std::is_convertible<U*, T*>::value>::type>
    RefPtr(RefPtr<U>&& r) noexcept : ptr_(r.ptr_) {
    r.ptr_ = nullptr;
  }

  ~RefPtr() {
    static_assert(!cr::subtle::IsRefCountPreferenceOverridden(
                      static_cast<T*>(nullptr), static_cast<T*>(nullptr)),
                  "It's unsafe to override the ref count preference."
                  " Please remove REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE"
                  " from subclasses.");
    if (ptr_)
      Release(ptr_);
  }

  T* get() const { return ptr_; }

  T& operator*() const {
    CR_DCHECK(ptr_);
    return *ptr_;
  }

  T* operator->() const {
    CR_DCHECK(ptr_);
    return ptr_;
  }

  RefPtr& operator=(std::nullptr_t) {
    reset();
    return *this;
  }

  RefPtr& operator=(T* p) { return *this = RefPtr(p); }

  // Unified assignment operator.
  RefPtr& operator=(RefPtr r) noexcept {
    swap(r);
    return *this;
  }

  // Sets managed object to null and releases reference to the previous managed
  // object, if it existed.
  void reset() { RefPtr().swap(*this); }

  // Returns the owned pointer (if any), releasing ownership to the caller. The
  // caller is responsible for managing the lifetime of the reference.
  T* release() CR_WARN_UNUSED_RESULT;

  void swap(RefPtr& r) noexcept { std::swap(ptr_, r.ptr_); }

  explicit operator bool() const { return ptr_ != nullptr; }

  template <typename U>
  bool operator==(const RefPtr<U>& rhs) const {
    return ptr_ == rhs.get();
  }

  template <typename U>
  bool operator!=(const RefPtr<U>& rhs) const {
    return !operator==(rhs);
  }

  template <typename U>
  bool operator<(const RefPtr<U>& rhs) const {
    return ptr_ < rhs.get();
  }

 protected:
  T* ptr_ = nullptr;

 private:
  template <typename U>
  friend RefPtr<U> cr::AdoptRef(U*);
  friend class ::cr::SequencedTaskRunner;

  // Friend access so these classes can use the constructor below as part of a
  // binary size optimization.
  friend class ::cr::internal::BasePromise;
  friend class ::cr::WrappedPromise;

  RefPtr(T* p, cr::subtle::AdoptRefTag) : ptr_(p) {}

  // Friend required for move constructors that set r.ptr_ to null.
  template <typename U>
  friend class RefPtr;

  // Non-inline helpers to allow:
  //     class Opaque;
  //     extern template class scoped_refptr<Opaque>;
  // Otherwise the compiler will complain that Opaque is an incomplete type.
  static void AddRef(T* ptr);
  static void Release(T* ptr);
};

template <typename T>
T* RefPtr<T>::release() {
  T* ptr = ptr_;
  ptr_ = nullptr;
  return ptr;
}

// static
template <typename T>
void RefPtr<T>::AddRef(T* ptr) {
  ptr->AddRef();
}

// static
template <typename T>
void RefPtr<T>::Release(T* ptr) {
  ptr->Release();
}

template <typename T, typename U>
bool operator==(const RefPtr<T>& lhs, const U* rhs) {
  return lhs.get() == rhs;
}

template <typename T, typename U>
bool operator==(const T* lhs, const RefPtr<U>& rhs) {
  return lhs == rhs.get();
}

template <typename T>
bool operator==(const RefPtr<T>& lhs, std::nullptr_t null) {
  return !static_cast<bool>(lhs);
}

template <typename T>
bool operator==(std::nullptr_t null, const RefPtr<T>& rhs) {
  return !static_cast<bool>(rhs);
}

template <typename T, typename U>
bool operator!=(const RefPtr<T>& lhs, const U* rhs) {
  return !operator==(lhs, rhs);
}

template <typename T, typename U>
bool operator!=(const T* lhs, const RefPtr<U>& rhs) {
  return !operator==(lhs, rhs);
}

template <typename T>
bool operator!=(const RefPtr<T>& lhs, std::nullptr_t null) {
  return !operator==(lhs, null);
}

template <typename T>
bool operator!=(std::nullptr_t null, const RefPtr<T>& rhs) {
  return !operator==(null, rhs);
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const RefPtr<T>& p) {
  return out << p.get();
}

template <typename T>
void swap(RefPtr<T>& lhs, RefPtr<T>& rhs) noexcept {
  lhs.swap(rhs);
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_PTR_H_

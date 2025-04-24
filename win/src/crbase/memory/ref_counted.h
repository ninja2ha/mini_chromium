// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_COUNTED_H_
#define MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_COUNTED_H_

#include <stddef.h>

#include <utility>

#include "crbase/atomic/atomic_ref_count.h"
#include "crbase/base_export.h"
#include "crbase/compiler_specific.h"
#include "crbase/logging.h"
#include "crbase/memory/ref_ptr.h"
#include "crbase/build_platform.h"

namespace cr {
namespace subtle {

class CRBASE_EXPORT RefCountedBase {
 public:
  bool HasOneRef() const { return ref_count_ == 1; }
  bool HasAtLeastOneRef() const { return ref_count_ >= 1; }

 protected:
  explicit RefCountedBase(StartRefCountFromZeroTag) {
  }

  explicit RefCountedBase(StartRefCountFromOneTag) : ref_count_(1) {
  }

  ~RefCountedBase() {
  }

  void AddRef() const {
    AddRefImpl();
  }

  // Returns true if the object should self-delete.
  bool Release() const {
    ReleaseImpl();
    return ref_count_ == 0;
  }

 private:
  template <typename U>
  friend cr::RefPtr<U> cr::AdoptRef(U*);

  void Adopted() const {
  }

#if defined(MINI_CHROMIUM_ARCH_CPU_64_BITS)
  void AddRefImpl() const;
  void ReleaseImpl() const;
#else
  void AddRefImpl() const { ++ref_count_; }
  void ReleaseImpl() const { --ref_count_; }
#endif

  mutable uint32_t ref_count_ = 0;
  static_assert(std::is_unsigned<decltype(ref_count_)>::value,
                "ref_count_ must be an unsigned type.");
};

class CRBASE_EXPORT RefCountedThreadSafeBase {
 public:
  bool HasOneRef() const;
  bool HasAtLeastOneRef() const;

 protected:
  explicit RefCountedThreadSafeBase(StartRefCountFromZeroTag) {}
  explicit RefCountedThreadSafeBase(StartRefCountFromOneTag)
      : ref_count_(1) {
  }

  ~RefCountedThreadSafeBase() = default;

// Release and AddRef are suitable for inlining on X86 because they generate
// very small code sequences. On other platforms (ARM), it causes a size
// regression and is probably not worth it.
#if defined(MINI_CHROMIUM_ARCH_CPU_X86_FAMILY)
  // Returns true if the object should self-delete.
  bool Release() const { return ReleaseImpl(); }
  void AddRef() const { AddRefImpl(); }
  void AddRefWithCheck() const { AddRefWithCheckImpl(); }
#else
  // Returns true if the object should self-delete.
  bool Release() const;
  void AddRef() const;
  void AddRefWithCheck() const;
#endif

 private:
  template <typename U>
  friend RefPtr<U> cr::AdoptRef(U*);

  void Adopted() const {
  }

  CR_ALWAYS_INLINE void AddRefImpl() const {
    ref_count_.Increment();
  }

  CR_ALWAYS_INLINE void AddRefWithCheckImpl() const {
    CR_CHECK(ref_count_.Increment() > 0);
  }

  CR_ALWAYS_INLINE bool ReleaseImpl() const {
    if (!ref_count_.Decrement()) {
      return true;
    }
    return false;
  }

  mutable AtomicRefCount ref_count_{0};
};

}  // namespace subtle

//
// A base class for reference counted classes.  Otherwise, known as a cheap
// knock-off of WebKit's RefCounted<T> class.  To use this, just extend your
// class from it like so:
//
//   class MyFoo : public cr::RefCounted<MyFoo> {
//    ...
//    private:
//     friend class cr::RefCounted<MyFoo>;
//     ~MyFoo();
//   };
//
// You should always make your destructor non-public, to avoid any code deleting
// the object accidently while there are references to it.
//
//
// The ref count manipulation to RefCounted is NOT thread safe and has DCHECKs
// to trap unsafe cross thread usage. A subclass instance of RefCounted can be
// passed to another execution sequence only when its ref count is 1. If the ref
// count is more than 1, the RefCounted class verifies the ref updates are made
// on the same execution sequence as the previous ones. The subclass can also
// manually call IsOnValidSequence to trap other non-thread-safe accesses; see
// the documentation for that method.
//
//
// The reference count starts from zero by default, and we intended to migrate
// to start-from-one ref count. Put CR_REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE;
// to the ref counted class to opt-in.
//
// If an object has start-from-one ref count, the first cr::RefPtr need to be
// created by cr::AdoptRef() or cr::MakeRefCounted(). We can use
// cr::MakeRefCounted() to create create both type of ref counted object.
//
// The motivations to use start-from-one ref count are:
//  - Start-from-one ref count doesn't need the ref count increment for the
//    first reference.
//  - It can detect an invalid object acquisition for a being-deleted object
//    that has zero ref count. That tends to happen on custom deleter that
//    delays the deletion.
//    TODO(tzik): Implement invalid acquisition detection.
//  - Behavior parity to Blink's WTF::RefCounted, whose count starts from one.
//    And start-from-one ref count is a step to merge WTF::RefCounted into
//    cr::RefCounted.
//
#define CR_REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE          \
  static constexpr ::cr::subtle::StartRefCountFromOneTag   \
      kRefCountPreference = ::cr::subtle::kStartRefCountFromOneTag

template <class T, typename Traits>
class RefCounted;

template <typename T>
struct DefaultRefCountedTraits {
  static void Destruct(const T* x) {
    RefCounted<T, DefaultRefCountedTraits>::DeleteInternal(x);
  }
};

template <class T, typename Traits = DefaultRefCountedTraits<T>>
class RefCounted : public subtle::RefCountedBase {
 public:
  static const subtle::StartRefCountFromZeroTag kRefCountPreference =
      subtle::kStartRefCountFromZeroTag;

  RefCounted(const RefCounted&) = delete;
  RefCounted& operator=(const RefCounted&) = delete;

  RefCounted() : subtle::RefCountedBase(T::kRefCountPreference) {}

  void AddRef() const {
    subtle::RefCountedBase::AddRef();
  }

  void Release() const {
    if (subtle::RefCountedBase::Release()) {
      // Prune the code paths which the static analyzer may take to simulate
      // object destruction. Use-after-free errors aren't possible given the
      // lifetime guarantees of the refcounting system.
      Traits::Destruct(static_cast<const T*>(this));
    }
  }

 protected:
  ~RefCounted() = default;

 private:
  friend struct DefaultRefCountedTraits<T>;
  template <typename U>
  static void DeleteInternal(const U* x) {
    delete x;
  }
};

// Forward declaration.
template <class T, typename Traits> class RefCountedThreadSafe;

// Default traits for RefCountedThreadSafe<T>.  Deletes the object when its ref
// count reaches 0.  Overload to delete it on a different thread etc.
template<typename T>
struct DefaultRefCountedThreadSafeTraits {
  static void Destruct(const T* x) {
    // Delete through RefCountedThreadSafe to make child classes only need to be
    // friend with RefCountedThreadSafe instead of this struct, which is an
    // implementation detail.
    RefCountedThreadSafe<T,
                         DefaultRefCountedThreadSafeTraits>::DeleteInternal(x);
  }
};

//
// A thread-safe variant of RefCounted<T>
//
//   class MyFoo : public base::RefCountedThreadSafe<MyFoo> {
//    ...
//   };
//
// If you're using the default trait, then you should add compile time
// asserts that no one else is deleting your object.  i.e.
//    private:
//     friend class cr::RefCountedThreadSafe<MyFoo>;
//     ~MyFoo();
//
// We can use REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE() with RefCountedThreadSafe
// too. See the comment above the RefCounted definition for details.
template <class T, typename Traits = DefaultRefCountedThreadSafeTraits<T> >
class RefCountedThreadSafe : public subtle::RefCountedThreadSafeBase {
 public:
  static constexpr subtle::StartRefCountFromZeroTag kRefCountPreference =
      subtle::kStartRefCountFromZeroTag;

  RefCountedThreadSafe(const RefCountedThreadSafe&) = delete;
  RefCountedThreadSafe& operator=(const RefCountedThreadSafe&) = delete;

  explicit RefCountedThreadSafe()
      : subtle::RefCountedThreadSafeBase(T::kRefCountPreference) {}

  void AddRef() const { AddRefImpl(T::kRefCountPreference); }

  void Release() const {
    if (subtle::RefCountedThreadSafeBase::Release()) {
      Traits::Destruct(static_cast<const T*>(this));
    }
  }

 protected:
  ~RefCountedThreadSafe() = default;

 private:
  friend struct DefaultRefCountedThreadSafeTraits<T>;
  template <typename U>
  static void DeleteInternal(const U* x) {
    delete x;
  }

  void AddRefImpl(subtle::StartRefCountFromZeroTag) const {
    subtle::RefCountedThreadSafeBase::AddRef();
  }

  void AddRefImpl(subtle::StartRefCountFromOneTag) const {
    subtle::RefCountedThreadSafeBase::AddRefWithCheck();
  }
};

//
// A thread-safe wrapper for some piece of data so we can place other
// things in scoped_refptrs<>.
//
template<typename T>
class RefCountedData
    : public cr::RefCountedThreadSafe< cr::RefCountedData<T> > {
 public:
  RefCountedData() : data() {}
  RefCountedData(const T& in_value) : data(in_value) {}
  RefCountedData(T&& in_value) : data(std::move(in_value)) {}

  T data;

 private:
  friend class cr::RefCountedThreadSafe<cr::RefCountedData<T> >;
  ~RefCountedData() = default;
};

template <typename T>
bool operator==(const RefCountedData<T>& lhs, const RefCountedData<T>& rhs) {
  return lhs.data == rhs.data;
}

template <typename T>
bool operator!=(const RefCountedData<T>& lhs, const RefCountedData<T>& rhs) {
  return !(lhs == rhs);
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_COUNTED_H_
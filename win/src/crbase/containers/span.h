/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 *  From: https://github.com/webrtc-sdk/webrtc/blob/m125_release/api/array_view.h
 *
 */

#ifndef MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_SPAN_H_
#define MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_SPAN_H_

#include <stdint.h>

#include <algorithm>
#include <array>
#include <vector>
#include <iterator>
#include <type_traits>

#include "crbase/logging.h"
#include "crbase/helper/template_util.h"

namespace cr {

// tl;dr: cr::Span is the same thing as gsl::span from the Guideline
//        Support Library.
//
// Many functions read from or write to arrays. The obvious way to do this is
// to use two arguments, a pointer to the first element and an element count:
//
//   bool Contains17(const int* arr, size_t size) {
//     for (size_t i = 0; i < size; ++i) {
//       if (arr[i] == 17)
//         return true;
//     }
//     return false;
//   }
//
// This is flexible, since it doesn't matter how the array is stored (C array,
// std::vector, cr::Buffer, ...), but it's error-prone because the caller 
// has to correctly specify the array length:
//
//   Contains17(arr, cr::size(arr));      // C array
//   Contains17(arr.data(), arr.size());  // std::vector
//   Contains17(arr, size);               // pointer + size
//   ...
//
// It's also kind of messy to have two separate arguments for what is
// conceptually a single thing.
//
// Enter cr::ArrayView<T>. It contains a T pointer (to an array it doesn't
// own) and a count, and supports the basic things you'd expect, such as
// indexing and iteration. It allows us to write our function like this:
//
//   bool Contains17(cr::Span<const int> arr) {
//     for (auto e : arr) {
//       if (e == 17)
//         return true;
//     }
//     return false;
//   }
//
// And even better, because a bunch of things will implicitly convert to
// ArrayView, we can call it like this:
//
//   Contains17(arr);                             // C array
//   Contains17(arr);                             // std::vector
//   Contains17(cr::Span<int>(arr, size));        // pointer + size
//   Contains17(nullptr);                         // nullptr -> empty ArrayView
//   ...
//
// Span<T> stores both a pointer and a size, but you may also use
// Span<T, N>, which has a size that's fixed at compile time (which means
// it only has to store the pointer).
//
// One important point is that Span<T> and Span<const T> are different types, 
// which allow and don't allow mutation of the array elements, respectively. 
// The implicit conversions work just like you'd hope, so that e.g. vector<int> 
// will convert to either Span<int> or Span<const int>, but const vector<int> 
// will convert only to Span<const int>.
// (Span itself can be the source type in such conversions, so Span<int> will 
// convert to Span<const int>.)
//
// Note: Span is tiny (just a pointer and a count if variable-sized, just a 
// pointer if fix-sized) and trivially copyable, so it's probably cheaper to
// pass it by value than by const reference.

namespace impl {

// Magic constant for indicating that the size of an ArrayView is variable
// instead of fixed.
enum : std::ptrdiff_t { kArrayViewVarSize = -4711 };

// Base class for Span of fixed nonzero size.
template <typename T, std::ptrdiff_t Size>
class SpanBase {
  static_assert(Size > 0, "Span size must be variable or non-negative");

 public:
   SpanBase(T* data, size_t size) : data_(data) {}

  static constexpr size_t size() { return Size; }
  static constexpr bool empty() { return false; }
  T* data() const { return data_; }

 protected:
  static constexpr bool fixed_size() { return true; }

 private:
  T* data_;
};

// Specialized base class for Span of fixed zero size.
template <typename T>
class SpanBase<T, 0> {
 public:
  explicit SpanBase(T* data, size_t size) {}

  static constexpr size_t size() { return 0; }
  static constexpr bool empty() { return true; }
  T* data() const { return nullptr; }

 protected:
  static constexpr bool fixed_size() { return true; }
};

// Specialized base class for Span of variable size.
template <typename T>
class SpanBase<T, impl::kArrayViewVarSize> {
 public:
  SpanBase(T* data, size_t size)
      : data_(size == 0 ? nullptr : data), size_(size) {}

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  T* data() const { return data_; }

 protected:
  static constexpr bool fixed_size() { return false; }

 private:
  T* data_;
  size_t size_;
};

}  // namespace impl

template <typename T, std::ptrdiff_t Size = impl::kArrayViewVarSize>
class Span final : public impl::SpanBase<T, Size> {
 public:
  using value_type = T;
  using const_iterator = const T*;

  // Construct an ArrayView from a pointer and a length.
  template <typename U>
  Span(U* data, size_t size)
      : impl::SpanBase<T, Size>::SpanBase(data, size) {
    CR_DCHECK((size == 0 ? nullptr : data) == this->data());
    CR_DCHECK(size == this->size());
    // data is null iff size == 0.
    CR_DCHECK((!this->data()) == (this->size() == 0)); 
  }

  // Construct an empty ArrayView. Note that fixed-size ArrayViews of size > 0
  // cannot be empty.
  Span() : Span(nullptr, 0) {}
  Span(std::nullptr_t)  // NOLINT
      : Span() {}
  Span(std::nullptr_t, size_t size)
      : Span(static_cast<T*>(nullptr), size) {
    static_assert(Size == 0 || Size == impl::kArrayViewVarSize, "");
    CR_DCHECK(0 == size);
  }

  // Construct an ArrayView from a C-style array.
  template <typename U, size_t N>
  Span(U (&array)[N])  // NOLINT
      : Span(array, N) {
    static_assert(Size == N || Size == impl::kArrayViewVarSize,
                  "Array size must match ArrayView size");
  }

  // (Only if size is fixed.) Construct a fixed size ArrayView<T, N> from a
  // non-const std::array instance. For an ArrayView with variable size, the
  // used ctor is Span(U& u) instead.
  template <typename U,
            size_t N,
            typename std::enable_if<
                Size == static_cast<std::ptrdiff_t>(N)>::type* = nullptr>
  Span(std::array<U, N>& u)  // NOLINT
      : Span(u.data(), u.size()) {}

  // (Only if size is fixed.) Construct a fixed size ArrayView<T, N> where T is
  // const from a const(expr) std::array instance. For an ArrayView with
  // variable size, the used ctor is ArrayView(U& u) instead.
  template <typename U,
            size_t N,
            typename std::enable_if<
                Size == static_cast<std::ptrdiff_t>(N)>::type* = nullptr>
  Span(const std::array<U, N>& u)  // NOLINT
      : Span(u.data(), u.size()) {}

  // (Only if size is fixed.) Construct an ArrayView from any type U that has a
  // static constexpr size() method whose return value is equal to Size, and a
  // data() method whose return value converts implicitly to T*. In particular,
  // this means we allow conversion from ArrayView<T, N> to ArrayView<const T,
  // N>, but not the other way around. We also don't allow conversion from
  // ArrayView<T> to ArrayView<T, N>, or from ArrayView<T, M> to ArrayView<T,
  // N> when M != N.
  template <
      typename U,
      typename std::enable_if<Size != impl::kArrayViewVarSize &&
                              has_data_and_size<U, T>::value>::type* = nullptr>
  Span(U& u)  // NOLINT
      : Span(u.data(), u.size()) {
    static_assert(U::size() == Size, "Sizes must match exactly");
  }
  template <
      typename U,
      typename std::enable_if<Size != impl::kArrayViewVarSize &&
                              has_data_and_size<U, T>::value>::type* = nullptr>
  Span(const U& u)  // NOLINT(runtime/explicit)
      : Span(u.data(), u.size()) {
    static_assert(U::size() == Size, "Sizes must match exactly");
  }

  // (Only if size is variable.) Construct an ArrayView from any type U that
  // has a size() method whose return value converts implicitly to size_t, and
  // a data() method whose return value converts implicitly to T*. In
  // particular, this means we allow conversion from ArrayView<T> to
  // ArrayView<const T>, but not the other way around. Other allowed
  // conversions include
  // Span<T, N> to Span<T> or Span<const T>,
  // std::vector<T> to Span<T> or Span<const T>,
  // const std::vector<T> to Span<const T>,
  // cr::Buffer to Span<uint8_t> or Span<const uint8_t>, and
  // const rtc::Buffer to ArrayView<const uint8_t>.
  template <
      typename U,
      typename std::enable_if<Size == impl::kArrayViewVarSize &&
                              has_data_and_size<U, T>::value>::type* = nullptr>
  Span(U& u)  // NOLINT
      : Span(u.data(), u.size()) {}
  template <
      typename U,
      typename std::enable_if<Size == impl::kArrayViewVarSize &&
                              has_data_and_size<U, T>::value>::type* = nullptr>
    Span(const U& u)  // NOLINT(runtime/explicit)
      : Span(u.data(), u.size()) {}

  // Indexing and iteration. These allow mutation even if the ArrayView is
  // const, because the ArrayView doesn't own the array. (To prevent mutation,
  // use a const element type.)
  T& operator[](size_t idx) const {
    CR_DCHECK(idx < this->size());
    CR_DCHECK(this->data());
    return this->data()[idx];
  }
  T* begin() const { return this->data(); }
  T* end() const { return this->data() + this->size(); }
  const T* cbegin() const { return this->data(); }
  const T* cend() const { return this->data() + this->size(); }
  std::reverse_iterator<T*> rbegin() const {
    return std::make_reverse_iterator(end());
  }
  std::reverse_iterator<T*> rend() const {
    return std::make_reverse_iterator(begin());
  }
  std::reverse_iterator<const T*> crbegin() const {
    return std::make_reverse_iterator(cend());
  }
  std::reverse_iterator<const T*> crend() const {
    return std::make_reverse_iterator(cbegin());
  }

  Span<T> subview(size_t offset, size_t size) const {
    return offset < this->size()
               ? Span<T>(this->data() + offset,
                              std::min(size, this->size() - offset))
               : Span<T>();
  }
  Span<T> subview(size_t offset) const {
    return subview(offset, this->size());
  }
};

// Comparing two ArrayViews compares their (pointer,size) pairs; it does *not*
// dereference the pointers.
template <typename T, std::ptrdiff_t Size1, std::ptrdiff_t Size2>
bool operator==(const Span<T, Size1>& a, const Span<T, Size2>& b) {
  return a.data() == b.data() && a.size() == b.size();
}
template <typename T, std::ptrdiff_t Size1, std::ptrdiff_t Size2>
bool operator!=(const Span<T, Size1>& a, const Span<T, Size2>& b) {
  return !(a == b);
}

// Variable-size Span are the size of two pointers; fixed-size ArrayViews
// are the size of one pointer. (And as a special case, fixed-size ArrayViews
// of size 0 require no storage.)
static_assert(sizeof(Span<int>) == 2 * sizeof(int*), "");
static_assert(sizeof(Span<int, 17>) == sizeof(int*), "");
static_assert(std::is_empty<Span<int, 0>>::value, "");

template <typename T>
inline Span<T> make_span(T* data, size_t size) {
  return Span<T>(data, size);
}

// adapts for stl.
template <typename U>
inline Span<typename const U::value_type> make_span(const U& value) {
  static_assert(cr::has_data_and_size<U, const void>::value,
                "class 'U' was required stl methods data() and size()");
  return Span<typename const U::value_type>(value.data(), value.size());
}

template<typename T>
inline Span<uint8_t> make_bytes_span(T* data, size_t size) {
  using writeable_type = std::remove_const<T>::type;
  return Span<uint8_t>(
      reinterpret_cast<uint8_t*>(const_cast<writeable_type*>(data)),
      size * sizeof(T));
}

// adapts for stl.
template <typename U>
inline Span<uint8_t> make_bytes_span(const U& value) {
  static_assert(cr::has_data_and_size<U, const void>::value, 
                "class 'U' was required stl methods data() and size()");
  using writeable_type = std::remove_const<typename U::value_type>::type;
  return Span<uint8_t>(
      reinterpret_cast<uint8_t*>(const_cast<writeable_type*>(value.data())),
      value.size() * sizeof(writeable_type));
}

// Only for primitive types that have the same size and aligment.
// Allow reinterpret cast of the array view to another primitive type of the
// same size.
// Template arguments order is (U, T, Size) to allow deduction of the template
// arguments in client calls: reinterpret_span<target_type>(view).
template <typename U, typename T, std::ptrdiff_t Size>
inline Span<U, Size> reinterpret_span(Span<T, Size> view) {
  static_assert(sizeof(U) == sizeof(T) && alignof(U) == alignof(T),
                "Span reinterpret_cast is only supported for casting "
                "between views that represent the same chunk of memory.");
  static_assert(
      std::is_fundamental<T>::value && std::is_fundamental<U>::value,
      "Span reinterpret_cast is only supported for casting between "
      "fundamental types.");
  return Span<U, Size>(reinterpret_cast<U*>(view.data()), view.size());
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_SPAN_H_
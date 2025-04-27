// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Derived from google3/util/gtl/stl_util

#ifndef MINI_CHROMIUM_SRC_CRBASE_HELPER_TEMPLATE_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_HELPER_TEMPLATE_UTIL_H_

namespace cr {

namespace internal {

template <typename...>
struct make_void {
  using type = void;
};

// A clone of C++17 std::void_t.
// Unlike the original version, we need |make_void| as a helper struct to avoid
// a C++14 defect.
// ref: http://en.cppreference.com/w/cpp/types/void_t
// ref: http://open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#1558
template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

// Used to detech whether the given type is an iterator.  This is normally used
// with std::enable_if to provide disambiguation for functions that take
// templatzed iterators as input.
template <typename T, typename = void>
struct is_iterator : std::false_type {};

template <typename T>
struct is_iterator<T,
    internal::void_t<typename std::iterator_traits<T>::iterator_category>>
    : std::true_type {};

}  // namespace internal

// Determines if the given class has zero-argument .data() and .size() methods
// whose return values are convertible to T* and size_t, respectively.
template <typename DS, typename T>
class has_data_and_size {
 private:
  template <
      typename C,
      typename std::enable_if<
          std::is_convertible<decltype(std::declval<C>().data()), T*>::value &&
          std::is_convertible<decltype(std::declval<C>().size()),
                              std::size_t>::value>::type* = nullptr>
  static int Test(int) {return int(0);}

  template <typename>
  static char Test(...) { return char(0); }

 public:
  static constexpr bool value = std::is_same<decltype(Test<DS>(0)), int>::value;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_HELPER_TEMPLATE_UTIL_H_

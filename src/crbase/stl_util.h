// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_STL_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_STL_UTIL_H_

#include <memory>
#include <initializer_list>
#include <array>
#include <string>

#include "crbase/internal/template_util.h"

namespace cr {

////////////////////////////////////////////////////////////////////////////////

// Used to explicitly mark the return value of a function as unused. If you are
// really sure you don't want to do anything with the return value of a function
// that has been marked CR_WARN_UNUSED_RESULT, wrap it with this. Example:
//
//   std::unique_ptr<MyType> my_var = ...;
//   if (TakeOwnership(my_var.get()) == SUCCESS)
//     cr::i(my_var.release());
//
template<typename T>
inline void ignore_result(const T&) {
}

////////////////////////////////////////////////////////////////////////////////

// C++14 implementation of C++17's std::size():
// http://en.cppreference.com/w/cpp/iterator/size
template <typename Container>
constexpr auto size(const Container& c) -> decltype(c.size()) {
  return c.size();
}

template <typename T, size_t N>
constexpr size_t size(const T (&array)[N]) noexcept {
  return N;
}

////////////////////////////////////////////////////////////////////////////////

// C++14 implementation of C++17's std::empty():
// http://en.cppreference.com/w/cpp/iterator/empty
template <typename Container>
constexpr auto empty(const Container& c) -> decltype(c.empty()) {
  return c.empty();
}

template <typename T, size_t N>
constexpr bool empty(const T (&array)[N]) noexcept {
  return false;
}

template <typename T>
constexpr bool empty(std::initializer_list<T> il) noexcept {
  return il.size() == 0;
}

////////////////////////////////////////////////////////////////////////////////

// Simplified C++14 implementation of  C++20's std::to_address.
// Note: This does not consider specializations of pointer_traits<>::to_address,
// since that member function may only be present in C++20 and later.
//
// Reference: https://wg21.link/pointer.conversion#lib:to_address
template <typename T>
constexpr T* to_address(T* p) noexcept {
  static_assert(!std::is_function<T>::value,
                "Error: T must not be a function type.");
  return p;
}

template <typename Ptr>
constexpr auto to_address(const Ptr& p) noexcept {
  return to_address(p.operator->());
}

////////////////////////////////////////////////////////////////////////////////

// C++14 implementation of C++17's std::data():
// http://en.cppreference.com/w/cpp/iterator/data
template <typename Container>
constexpr auto data(Container& c) -> decltype(c.data()) {
  return c.data();
}

// std::basic_string::data() had no mutable overload prior to C++17 [1].
// Hence this overload is provided.
// Note: str[0] is safe even for empty strings, as they are guaranteed to be
// null-terminated [2].
//
// [1] http://en.cppreference.com/w/cpp/string/basic_string/data
// [2] http://en.cppreference.com/w/cpp/string/basic_string/operator_at
template <typename CharT, typename Traits, typename Allocator>
CharT* data(std::basic_string<CharT, Traits, Allocator>& str) {
  return std::addressof(str[0]);
}

template <typename Container>
constexpr auto data(const Container& c) -> decltype(c.data()) {
  return c.data();
}

template <typename T, size_t N>
constexpr T* data(T (&array)[N]) noexcept {
  return array;
}

template <typename T>
constexpr const T* data(std::initializer_list<T> il) noexcept {
  return il.begin();
}

// std::array::data() was not constexpr prior to C++17 [1].
// Hence these overloads are provided.
//
// [1] https://en.cppreference.com/w/cpp/container/array/data
template <typename T, size_t N>
constexpr T* data(std::array<T, N>& array) noexcept {
  return !array.empty() ? &array[0] : nullptr;
}

template <typename T, size_t N>
constexpr const T* data(const std::array<T, N>& array) noexcept {
  return !array.empty() ? &array[0] : nullptr;
}

////////////////////////////////////////////////////////////////////////////////

// General purpose implementation to check if |container| contains |value|.
template <typename Container,
          typename Value,
          std::enable_if_t<
              !internal::has_find_with_npos<Container, Value>::value &&
              !internal::has_find_with_end<Container, Value>::value &&
              !internal::has_contains<Container, Value>::value>* = nullptr>
bool Contains(const Container& container, const Value& value) {
  using std::begin;
  using std::end;
  return std::find(begin(container), end(container), value) != end(container);
}

// Specialized Contains() implementation for when |container| has a find()
// member function and a static npos member, but no contains() member function.
template <typename Container,
          typename Value,
          std::enable_if_t<internal::has_find_with_npos<Container, 
                                                        Value>::value &&
                           !internal::has_contains<Container, Value>::value>* =
              nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.find(value) != Container::npos;
}

// Specialized Contains() implementation for when |container| has a find()
// and end() member function, but no contains() member function.
template <typename Container,
          typename Value,
          std::enable_if_t<
              internal::has_find_with_end<Container, Value>::value &&
              !internal::has_contains<Container, Value>::value>* =
                  nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.find(value) != container.end();
}

// Specialized Contains() implementation for when |container| has a contains()
// member function.
template <
    typename Container,
    typename Value,
    std::enable_if_t<internal::has_contains<Container, Value>::value>* 
                         = nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.contains(value);
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STL_UTIL_H_
// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_INTERNAL_TEMPLATE_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_INTERNAL_TEMPLATE_UTIL_H_

#include <type_traits>

namespace cr {
namespace internal {

////////////////////////////////////////////////////////////////////////////////
// Implementation of C++17's std::void_t.

// We use |::cr::internal::make_void| as a helper struct to avoid a C++14
// defect:
//   http://en.cppreference.com/w/cpp/types/void_t
//   http://open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#1558
template <typename...>
struct make_void {
  using type = void;
};

// cr::internal::void_t is an implementation of std::void_t from C++17.
template <typename... Ts>
using void_t = typename ::cr::internal::make_void<Ts...>::type;


////////////////////////////////////////////////////////////////////////////////
// Utility type traits used for specializing cr::Contains() below.
// cr::Contains() implements in the cr_base/containers/container_util.h

// For std::basic_string
template <typename Container, typename Element, typename = void>
struct has_find_with_npos : std::false_type {};

template <typename Container, typename Element>
struct has_find_with_npos<
    Container,
    Element,
    ::cr::internal::void_t<decltype(std::declval<const Container&>().find(
        std::declval<const Element&>()) != Container::npos)>>
    : std::true_type {};

// For std::map | std::unordered_map
template <typename Container, typename Element, typename = void>
struct has_find_with_end : std::false_type {};

template <typename Container, typename Element>
struct has_find_with_end<
    Container,
    Element,
    ::cr::internal::void_t<decltype(std::declval<const Container&>().find(
        std::declval<const Element&>()) !=
        std::declval<const Container&>().end())>>
    : std::true_type {};

// 
template <typename Container, typename Element, typename = void>
struct has_contains : std::false_type {};

template <typename Container, typename Element>
struct has_contains<
    Container,
    Element,
    ::cr::internal::void_t<decltype(std::declval<const Container&>().contains(
        std::declval<const Element&>()))>> : std::true_type {};

}  // namesapce internal
}  // namesapce cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_INTERNAL_TEMPLATE_UTIL_H_
// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_INTERNAL_TEMPLATE_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_INTERNAL_TEMPLATE_UTIL_H_

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
// cr::Contains() implements in the cr_base/stl_util.h

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

////////////////////////////////////////////////////////////////////////////////

// Implementation of C++20's std::remove_cvref.
//
// References:
// - https://en.cppreference.com/w/cpp/types/remove_cvref
// - https://wg21.link/meta.trans.other#lib:remove_cvref
template <typename T>
struct remove_cvref {
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

// Implementation of C++20's std::remove_cvref_t.
//
// References:
// - https://en.cppreference.com/w/cpp/types/remove_cvref
// - https://wg21.link/meta.type.synop#lib:remove_cvref_t
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

////////////////////////////////////////////////////////////////////////////////
// C++14 implementation of C++17's std::disjunction.
//
// Used in below files: 
//   1. cr_base/containers/internal/contiguous_iterator.h
//
// Reference: https://en.cppreference.com/w/cpp/types/disjunction
// Specification: https://wg21.link/meta.logical#itemdecl:2
template <typename...>
struct disjunction : std::false_type {};

template <typename B1>
struct disjunction<B1> : B1 {};

template <typename B1, typename... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<static_cast<bool>(B1::value), B1, disjunction<Bn...>> {
};

////////////////////////////////////////////////////////////////////////////////
// Simplified implementation of C++20's std::iter_value_t.
//
// Used in below files: 
//   1. cr_base/containers/internal/contiguous_iterator.h
//
// As opposed to std::iter_value_t, this implementation does not restrict
// the type of `Iter` and does not consider specializations of
// `indirectly_readable_traits`.
//
// Reference: https://wg21.link/readable.traits#2
template <typename Iter>
using iter_value_t =  
    typename std::iterator_traits<remove_cvref_t<Iter>>::value_type;

////////////////////////////////////////////////////////////////////////////////
// Simplified implementation of C++20's std::iter_reference_t.
//
// Used in below files: 
//   1. cr_base/containers/span.h
//
// As opposed to std::iter_reference_t, this implementation does not restrict
// the type of `Iter`.
//
// Reference: https://wg21.link/iterator.synopsis#:~:text=iter_reference_t
template <typename Iter>
using iter_reference_t = decltype(*std::declval<Iter&>());

////////////////////////////////////////////////////////////////////////////////
// C++14 implementation of C++17's std::conjunction.
//
// Used in below files: 
//   1. cr_base/containers/internal/contiguous_iterator.h
//
// Reference: https://en.cppreference.com/w/cpp/types/conjunction
// Specification: https://wg21.link/meta.logical#1.itemdecl:1
template <typename...>
struct conjunction : std::true_type {};

template <typename B1>
struct conjunction<B1> : B1 {};

template <typename B1, typename... Bn>
struct conjunction<B1, Bn...>
  : std::conditional_t<static_cast<bool>(B1::value), conjunction<Bn...>, B1> {
};


////////////////////////////////////////////////////////////////////////////////
// C++14 implementation of C++17's std::bool_constant.
//
// Used for cr::internal::negation
//
// Reference: https://en.cppreference.com/w/cpp/types/integral_constant
// Specification: https://wg21.link/meta.type.synop
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

////////////////////////////////////////////////////////////////////////////////
// C++14 implementation of C++17's std::negation.
//
// Used in below files: 
//   1. cr_base/containers/internal/contiguous_iterator.h

// Reference: https://en.cppreference.com/w/cpp/types/negation
// Specification: https://wg21.link/meta.logical#itemdecl:3
template <typename B>
struct negation : bool_constant<!static_cast<bool>(B::value)> {};

////////////////////////////////////////////////////////////////////////////////

// Helper to express preferences in an overload set. If more than one overload
// are available for a given set of parameters the overload with the higher
// priority will be chosen.
template <size_t I>
struct priority_tag : priority_tag<I - 1> {};

template <>
struct priority_tag<0> {};

}  // namesapce internal
}  // namesapce cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_INTERNAL_TEMPLATE_UTIL_H_
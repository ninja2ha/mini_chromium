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

// Implementation detail of cr::void_t below.
template <typename...>
struct make_void {
  using type = void;
};

// cr::internal::void_t is an implementation of std::void_t from C++17.
//
// We use |cr::internal::make_void| as a helper struct to avoid a C++14
// defect:
//   http://en.cppreference.com/w/cpp/types/void_t
//   http://open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#1558
template <typename... Ts>
using void_t = typename ::cr::internal::make_void<Ts...>::type;

////////////////////////////////////////////////////////////////////////////////

// cr::internal::in_place_t is an implementation of std::in_place_t from
// C++17. A tag type used to request in-place construction in template vararg
// constructors.

// Specification:
// https://en.cppreference.com/w/cpp/utility/in_place
struct in_place_t {};
constexpr in_place_t in_place = {};

// cr::internal::in_place_type_t is an implementation of std::in_place_type_t 
// from C++17. 
// A tag type used for in-place construction when the type to construct needs 
// to be specified, such as with cr::unique_any, designed to be a drop-in 
// replacement.

// Specification:
// http://en.cppreference.com/w/cpp/utility/in_place
template <typename T>
struct in_place_type_t {};

template <typename T>
struct is_in_place_type_t {
  static constexpr bool value = false;
};

template <typename... Ts>
struct is_in_place_type_t<in_place_type_t<Ts...>> {
  static constexpr bool value = true;
};

////////////////////////////////////////////////////////////////////////////////

// C++14 implementation of C++17's std::bool_constant.
//
// Reference: https://en.cppreference.com/w/cpp/types/integral_constant
// Specification: https://wg21.link/meta.type.synop
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

////////////////////////////////////////////////////////////////////////////////

// C++14 implementation of C++17's std::conjunction.
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

// C++14 implementation of C++17's std::disjunction.
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

// C++14 implementation of C++17's std::negation.
//
// Reference: https://en.cppreference.com/w/cpp/types/negation
// Specification: https://wg21.link/meta.logical#itemdecl:3
template <typename B>
struct negation : bool_constant<!static_cast<bool>(B::value)> {};

////////////////////////////////////////////////////////////////////////////////

// Implementation of C++17's invoke_result.
//
// This implementation adds references to `Functor` and `Args` to work around
// some quirks of std::result_of. See the #Notes section of [1] for details.
//
// References:
// [1] https://en.cppreference.com/w/cpp/types/result_of
// [2] https://wg21.link/meta.trans.other#lib:invoke_result
template <typename Functor, typename... Args>
using invoke_result = std::result_of<Functor && (Args && ...)>;

// Implementation of C++17's std::invoke_result_t.
//
// Reference: https://wg21.link/meta.type.synop#lib:invoke_result_t
template <typename Functor, typename... Args>
using invoke_result_t = typename invoke_result<Functor, Args...>::type;

////////////////////////////////////////////////////////////////////////////////

// Refers at crbase/containers/internal/contiguous_iterator.h

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

// Refers at crbase/containers/internal/contiguous_iterator.h

// Simplified implementation of C++20's std::iter_value_t.
// As opposed to std::iter_value_t, this implementation does not restrict
// the type of `Iter` and does not consider specializations of
// `indirectly_readable_traits`.
//
// Reference: https://wg21.link/readable.traits#2
template <typename Iter>
using iter_value_t =
typename std::iterator_traits<remove_cvref_t<Iter>>::value_type;

////////////////////////////////////////////////////////////////////////////////

// Refers at crbase/containers/span.h

// Simplified implementation of C++20's std::iter_reference_t.
// As opposed to std::iter_reference_t, this implementation does not restrict
// the type of `Iter`.
//
// Reference: https://wg21.link/iterator.synopsis#:~:text=iter_reference_t
template <typename Iter>
using iter_reference_t = decltype(*std::declval<Iter&>());

////////////////////////////////////////////////////////////////////////////////

// Used to detech whether the given type is an iterator.  This is normally used
// with std::enable_if to provide disambiguation for functions that take
// templatzed iterators as input.
template <typename T, typename = void>
struct is_iterator : std::false_type {};

template <typename T>
struct is_iterator<T,
                   internal::void_t<
                       typename std::iterator_traits<T>::iterator_category>>
    : std::true_type {};

////////////////////////////////////////////////////////////////////////////////

// Helper to express preferences in an overload set. If more than one overload
// are available for a given set of parameters the overload with the higher
// priority will be chosen.
template <size_t I>
struct priority_tag : priority_tag<I - 1> {};

template <>
struct priority_tag<0> {};

////////////////////////////////////////////////////////////////////////////////

// Refers at crbase/containers/optional.h

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ <= 7
// Workaround for g++7 and earlier family.
// Due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80654, without this
// Optional<std::vector<T>> where T is non-copyable causes a compile error.
// As we know it is not trivially copy constructible, explicitly declare so.
template <typename T>
struct is_trivially_copy_constructible
    : std::is_trivially_copy_constructible<T> {};

template <typename... T>
struct is_trivially_copy_constructible<std::vector<T...>> : std::false_type {};
#else
// Otherwise use std::is_trivially_copy_constructible as is.
template <typename T>
using is_trivially_copy_constructible = std::is_trivially_copy_constructible<T>;
#endif

////////////////////////////////////////////////////////////////////////////////


template <typename Collection>
class has_key_type {
  template <typename C>
  static std::true_type test(typename C::key_type*);
  template <typename C>
  static std::false_type test(...);

 public:
  static constexpr bool value = decltype(test<Collection>(nullptr))::value;
};

// Utility type traits used for specializing cr::Contains() below.
template <typename Container, typename Element, typename = void>
struct has_find_with_npos : std::false_type {};

template <typename Container, typename Element>
struct has_find_with_npos<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().find(
                        std::declval<const Element&>()) != Container::npos)>>
    : std::true_type {};

template <typename Container, typename Element, typename = void>
struct has_find_with_end : std::false_type {};

template <typename Container, typename Element>
struct has_find_with_end<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().find(
        std::declval<const Element&>()) !=
        std::declval<const Container&>().end())>>
    : std::true_type {};

template <typename Container, typename Element, typename = void>
struct has_contains : std::false_type {};

template <typename Container, typename Element>
struct has_contains<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().contains(
        std::declval<const Element&>()))>> : std::true_type {};



}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_INTERNAL_TEMPLATE_UTIL_H_
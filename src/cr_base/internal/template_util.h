// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_INTERNAL_TEMPLATE_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_INTERNAL_TEMPLATE_UTIL_H_

#include <type_traits>

// Some versions of libstdc++ have partial support for type_traits, but misses
// a smaller subset while removing some of the older non-standard stuff. Assume
// that all versions below 5.0 fall in this category, along with one 5.0
// experimental release. Test for this by consulting compiler major version,
// the only reliable option available, so theoretically this could fail should
// you attempt to mix an earlier version of libstdc++ with >= GCC5. But
// that's unlikely to work out, especially as GCC5 changed ABI.
#define CR_GLIBCXX_5_0_0 20150123
#if (defined(__GNUC__) && __GNUC__ < 5) || \
    (defined(__GLIBCXX__) && __GLIBCXX__ == CR_GLIBCXX_5_0_0)
#define CR_USE_FALLBACKS_FOR_OLD_EXPERIMENTAL_GLIBCXX
#endif

// This hacks around using gcc with libc++ which has some incompatibilies.
// - is_trivially_* doesn't work: https://llvm.org/bugs/show_bug.cgi?id=27538
// TODO(danakj): Remove this when android builders are all using a newer version
// of gcc, or the android ndk is updated to a newer libc++ that works with older
// gcc versions.
#if !defined(__clang__) && defined(_LIBCPP_VERSION)
#define CR_USE_FALLBACKS_FOR_GCC_WITH_LIBCXX
#endif

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
//
// Used in below files: 
//   1. cr_base/data_stream/byte_buffer.h
//
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
  static int Test(int) {}

  template <typename>
  static char Test(...) {}

 public:
  static constexpr bool value = std::is_same<decltype(Test<DS>(0)), int>::value;
};


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

////////////////////////////////////////////////////////////////////////////////
// Used in below files: 
//   1. cr_base/containers/optional.h
//
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
// Used in below files: 
//   1. cr_base/containers/optional.h
//
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
// Used in below files: 
//   1. cr_base/containers/internal/vector_buffer.h
//
// is_trivially_copyable is especially hard to get right.
// - Older versions of libstdc++ will fail to have it like they do for other
//   type traits. This has become a subset of the second point, but used to be
//   handled independently.
// - An experimental release of gcc includes most of type_traits but misses
//   is_trivially_copyable, so we still have to avoid using libstdc++ in this
//   case, which is covered by CR_USE_FALLBACKS_FOR_OLD_EXPERIMENTAL_GLIBCXX.
// - When compiling libc++ from before r239653, with a gcc compiler, the
//   std::is_trivially_copyable can fail. So we need to work around that by not
//   using the one in libc++ in this case. This is covered by the
//   CR_USE_FALLBACKS_FOR_GCC_WITH_LIBCXX define, and is discussed in
//   https://llvm.org/bugs/show_bug.cgi?id=27538#c1 where they point out that
//   in libc++'s commit r239653 this is fixed by libc++ checking for gcc 5.1.
// - In both of the above cases we are using the gcc compiler. When defining
//   this ourselves on compiler intrinsics, the __is_trivially_copyable()
//   intrinsic is not available on gcc before version 5.1 (see the discussion in
//   https://llvm.org/bugs/show_bug.cgi?id=27538#c1 again), so we must check for
//   that version.
// - When __is_trivially_copyable() is not available because we are on gcc older
//   than 5.1, we need to fall back to something, so we use __has_trivial_copy()
//   instead based on what was done one-off in bit_cast() previously.

// TODO(crbug.com/554293): Remove this when all platforms have this in the std
// namespace and it works with gcc as needed.
#if defined(CR_USE_FALLBACKS_FOR_OLD_EXPERIMENTAL_GLIBCXX) || \
    defined(CR_USE_FALLBACKS_FOR_GCC_WITH_LIBCXX)
template <typename T>
struct is_trivially_copyable {
// TODO(danakj): Remove this when android builders are all using a newer version
// of gcc, or the android ndk is updated to a newer libc++ that does this for
// us.
#if _GNUC_VER >= 501
  static constexpr bool value = __is_trivially_copyable(T);
#else
  static constexpr bool value =
      __has_trivial_copy(T) && __has_trivial_destructor(T);
#endif
};
#else
template <class T>
using is_trivially_copyable = std::is_trivially_copyable<T>;
#endif


////////////////////////////////////////////////////////////////////////////////
// Used in below files: 
//   1. cr_base/containers/circular_deque.h
//
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


}  // namesapce internal
}  // namesapce cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_INTERNAL_TEMPLATE_UTIL_H_
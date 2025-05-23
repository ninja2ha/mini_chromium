// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Derived from google3/util/gtl/stl_util

#ifndef MINI_CHROMIUM_SRC_CRBASE_HELPER_STL_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_HELPER_STL_UTIL_H_

#include <algorithm>
#include <deque>
#include <forward_list>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "crbase/logging.h"
#include "crbase/helper/template_util.h"

namespace cr {

namespace internal {

// Calls erase on iterators of matching elements.
template <typename Container, typename Predicate>
void IterateAndEraseIf(Container& container, Predicate pred) {
  for (auto it = container.begin(); it != container.end();) {
    if (pred(*it))
      it = container.erase(it);
    else
      ++it;
  }
}

}  // namespace internal

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

// Used to explicitly mark the return value of a function as unused. If you are
// really sure you don't want to do anything with the return value of a function
// that has been marked WARN_UNUSED_RESULT, wrap it with this. Example:
//
//   std::unique_ptr<MyType> my_var = ...;
//   if (TakeOwnership(my_var.get()) == SUCCESS)
//     cr::ignore_result(my_var.release());
//
template<typename T>
inline void ignore_result(const T&) {
}

// Clears internal memory of an STL object.
// STL clear()/reserve(0) does not always free internal memory allocated
// This function uses swap/destructor to ensure the internal memory is freed.
template<class T>
void STLClearObject(T* obj) {
  T tmp;
  tmp.swap(*obj);
  // Sometimes "T tmp" allocates objects with memory (arena implementation?).
  // Hence using additional reserve(0) even if it doesn't always work.
  obj->reserve(0);
}

// Counts the number of instances of val in a container.
template <typename Container, typename T>
typename std::iterator_traits<
    typename Container::const_iterator>::difference_type
STLCount(const Container& container, const T& val) {
  return std::count(container.begin(), container.end(), val);
}

// Return a mutable char* pointing to a string's internal buffer,
// which may not be null-terminated. Writing through this pointer will
// modify the string.
//
// string_as_array(&str)[i] is valid for 0 <= i < str.size() until the
// next call to a string method that invalidates iterators.
//
// As of 2006-04, there is no standard-blessed way of getting a
// mutable reference to a string's internal buffer. However, issue 530
// (http://www.open-std.org/JTC1/SC22/WG21/docs/lwg-active.html#530)
// proposes this as the method. According to Matt Austern, this should
// already work on all current implementations.
inline char* string_as_array(std::string* str) {
  // DO NOT USE const_cast<char*>(str->data())
  return str->empty() ? NULL : &*str->begin();
}

// Test to see if a set or map contains a particular key.
// Returns true if the key is in the collection.
template <typename Collection, typename Key>
bool ContainsKey(const Collection& collection, const Key& key) {
  return collection.find(key) != collection.end();
}

namespace internal {

template <typename Collection>
class HasKeyType {
  template <typename C>
  static std::true_type test(typename C::key_type*);
  template <typename C>
  static std::false_type test(...);

 public:
  static constexpr bool value = decltype(test<Collection>(nullptr))::value;
};

// Utility type traits used for specializing base::Contains() below.
template <typename Container, typename Element, typename = void>
struct HasFindWithNpos : std::false_type {};

template <typename Container, typename Element>
struct HasFindWithNpos<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().find(
                        std::declval<const Element&>()) != Container::npos)>>
    : std::true_type {};

template <typename Container, typename Element, typename = void>
struct HasFindWithEnd : std::false_type {};

template <typename Container, typename Element>
struct HasFindWithEnd<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().find(
        std::declval<const Element&>()) !=
        std::declval<const Container&>().end())>>
    : std::true_type {};

template <typename Container, typename Element, typename = void>
struct HasContains : std::false_type {};

template <typename Container, typename Element>
struct HasContains<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().contains(
        std::declval<const Element&>()))>> : std::true_type {};

}  // namespace internal

// 
// Test to see if a collection like a vector contains a particular value.
// Returns true if the value is in the collection.
// Don't use this on collections such as sets or maps. This is enforced by
// disabling this method if the collection defines a key_type.
///template <typename Collection,
///          typename Value,
///          typename std::enable_if<!internal::HasKeyType<Collection>::value,
///                                  int>::type = 0>
///bool ContainsValue(const Collection& collection, const Value& value) {
///  return std::find(std::begin(collection), std::end(collection), value) !=
///         std::end(collection);
///}

// General purpose implementation to check if |container| contains |value|.
template <typename Container,
          typename Value,
          std::enable_if_t<
              !internal::HasFindWithNpos<Container, Value>::value &&
              !internal::HasFindWithEnd<Container, Value>::value &&
              !internal::HasContains<Container, Value>::value>* = nullptr>
bool Contains(const Container& container, const Value& value) {
  using std::begin;
  using std::end;
  return std::find(begin(container), end(container), value) != end(container);
}

// Specialized Contains() implementation for when |container| has a find()
// member function and a static npos member, but no contains() member function.
template <typename Container,
          typename Value,
          std::enable_if_t<internal::HasFindWithNpos<Container, Value>::value &&
                           !internal::HasContains<Container, Value>::value>* =
              nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.find(value) != Container::npos;
}

// Specialized Contains() implementation for when |container| has a find()
// and end() member function, but no contains() member function.
template <typename Container,
          typename Value,
          std::enable_if_t<internal::HasFindWithEnd<Container, Value>::value &&
                           !internal::HasContains<Container, Value>::value>* =
              nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.find(value) != container.end();
}

// Specialized Contains() implementation for when |container| has a contains()
// member function.
template <
    typename Container,
    typename Value,
    std::enable_if_t<internal::HasContains<Container, Value>::value>* = nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.contains(value);
}

// Returns true if the container is sorted.
template <typename Container>
bool STLIsSorted(const Container& cont) {
  // Note: Use reverse iterator on container to ensure we only require
  // value_type to implement operator<.
  return std::adjacent_find(cont.rbegin(), cont.rend(),
                            std::less<typename Container::value_type>())
      == cont.rend();
}

// Returns a new ResultType containing the difference of two sorted containers.
template <typename ResultType, typename Arg1, typename Arg2>
ResultType STLSetDifference(const Arg1& a1, const Arg2& a2) {
  CR_DCHECK(STLIsSorted(a1));
  CR_DCHECK(STLIsSorted(a2));
  ResultType difference;
  std::set_difference(a1.begin(), a1.end(),
                      a2.begin(), a2.end(),
                      std::inserter(difference, difference.end()));
  return difference;
}

// Returns a new ResultType containing the union of two sorted containers.
template <typename ResultType, typename Arg1, typename Arg2>
ResultType STLSetUnion(const Arg1& a1, const Arg2& a2) {
  CR_DCHECK(STLIsSorted(a1));
  CR_DCHECK(STLIsSorted(a2));
  ResultType result;
  std::set_union(a1.begin(), a1.end(),
                 a2.begin(), a2.end(),
                 std::inserter(result, result.end()));
  return result;
}

// Returns a new ResultType containing the intersection of two sorted
// containers.
template <typename ResultType, typename Arg1, typename Arg2>
ResultType STLSetIntersection(const Arg1& a1, const Arg2& a2) {
  CR_DCHECK(STLIsSorted(a1));
  CR_DCHECK(STLIsSorted(a2));
  ResultType result;
  std::set_intersection(a1.begin(), a1.end(),
                        a2.begin(), a2.end(),
                        std::inserter(result, result.end()));
  return result;
}

// Returns true if the sorted container |a1| contains all elements of the sorted
// container |a2|.
template <typename Arg1, typename Arg2>
bool STLIncludes(const Arg1& a1, const Arg2& a2) {
  CR_DCHECK(STLIsSorted(a1));
  CR_DCHECK(STLIsSorted(a2));
  return std::includes(a1.begin(), a1.end(),
                       a2.begin(), a2.end());
}

// Erase/EraseIf are based on library fundamentals ts v2 erase/erase_if
// http://en.cppreference.com/w/cpp/experimental/lib_extensions_2
// They provide a generic way to erase elements from a container.
// The functions here implement these for the standard containers until those
// functions are available in the C++ standard.
// For Chromium containers overloads should be defined in their own headers
// (like standard containers).
// Note: there is no std::erase for standard associative containers so we don't
// have it either.

template <typename CharT, typename Traits, typename Allocator, typename Value>
void Erase(std::basic_string<CharT, Traits, Allocator>& container,
           const Value& value) {
  container.erase(std::remove(container.begin(), container.end(), value),
                  container.end());
}

template <typename CharT, typename Traits, typename Allocator, class Predicate>
void EraseIf(std::basic_string<CharT, Traits, Allocator>& container,
             Predicate pred) {
  container.erase(std::remove_if(container.begin(), container.end(), pred),
                  container.end());
}

template <class T, class Allocator, class Value>
void Erase(std::deque<T, Allocator>& container, const Value& value) {
  container.erase(std::remove(container.begin(), container.end(), value),
                  container.end());
}

template <class T, class Allocator, class Predicate>
void EraseIf(std::deque<T, Allocator>& container, Predicate pred) {
  container.erase(std::remove_if(container.begin(), container.end(), pred),
                  container.end());
}

template <class T, class Allocator, class Value>
void Erase(std::vector<T, Allocator>& container, const Value& value) {
  container.erase(std::remove(container.begin(), container.end(), value),
                  container.end());
}

template <class T, class Allocator, class Predicate>
void EraseIf(std::vector<T, Allocator>& container, Predicate pred) {
  container.erase(std::remove_if(container.begin(), container.end(), pred),
                  container.end());
}

template <class T, class Allocator, class Value>
void Erase(std::forward_list<T, Allocator>& container, const Value& value) {
  // Unlike std::forward_list::remove, this function template accepts
  // heterogeneous types and does not force a conversion to the container's
  // value type before invoking the == operator.
  container.remove_if([&](const T& cur) { return cur == value; });
}

template <class T, class Allocator, class Predicate>
void EraseIf(std::forward_list<T, Allocator>& container, Predicate pred) {
  container.remove_if(pred);
}

template <class T, class Allocator, class Value>
void Erase(std::list<T, Allocator>& container, const Value& value) {
  // Unlike std::list::remove, this function template accepts heterogeneous
  // types and does not force a conversion to the container's value type before
  // invoking the == operator.
  container.remove_if([&](const T& cur) { return cur == value; });
}

template <class T, class Allocator, class Predicate>
void EraseIf(std::list<T, Allocator>& container, Predicate pred) {
  container.remove_if(pred);
}

template <class Key, class T, class Compare, class Allocator, class Predicate>
void EraseIf(std::map<Key, T, Compare, Allocator>& container, Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

template <class Key, class T, class Compare, class Allocator, class Predicate>
void EraseIf(std::multimap<Key, T, Compare, Allocator>& container,
             Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

template <class Key, class Compare, class Allocator, class Predicate>
void EraseIf(std::set<Key, Compare, Allocator>& container, Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

template <class Key, class Compare, class Allocator, class Predicate>
void EraseIf(std::multiset<Key, Compare, Allocator>& container,
             Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

template <class Key,
          class T,
          class Hash,
          class KeyEqual,
          class Allocator,
          class Predicate>
void EraseIf(std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& container,
             Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

template <class Key,
          class T,
          class Hash,
          class KeyEqual,
          class Allocator,
          class Predicate>
void EraseIf(
    std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& container,
    Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

template <class Key,
          class Hash,
          class KeyEqual,
          class Allocator,
          class Predicate>
void EraseIf(std::unordered_set<Key, Hash, KeyEqual, Allocator>& container,
             Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

template <class Key,
          class Hash,
          class KeyEqual,
          class Allocator,
          class Predicate>
void EraseIf(std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& container,
             Predicate pred) {
  internal::IterateAndEraseIf(container, pred);
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_HELPER_STL_UTIL_H_

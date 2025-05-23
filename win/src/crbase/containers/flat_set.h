// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_FLAT_SET_H_
#define MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_FLAT_SET_H_

#include <functional>

#include "crbase/containers/flat_tree.h"

namespace cr {

// flat_set is a container with a std::set-like interface that stores its
// contents in a sorted vector.
//
// Please see //base/containers/README.md for an overview of which container
// to select.
//
// PROS
//
//  - Good memory locality.
//  - Low overhead, especially for smaller sets.
//  - Performance is good for more workloads than you might expect (see
//    overview link above).
//
// CONS
//
//  - Inserts and removals are O(n).
//
// IMPORTANT NOTES
//
//  - Iterators are invalidated across mutations.
//  - If possible, construct a flat_set in one operation by inserting into
//    a std::vector and moving that vector into the flat_set constructor.
//  - For multiple removals use base::EraseIf() which is O(n) rather than
//    O(n * removed_items).
//
// QUICK REFERENCE
//
// Most of the core functionality is inherited from flat_tree. Please see
// flat_tree.h for more details for most of these functions. As a quick
// reference, the functions available are:
//
// Constructors (inputs need not be sorted):
//   flat_set(InputIterator first, InputIterator last,
//            FlatContainerDupes, const Compare& compare = Compare());
//   flat_set(const flat_set&);
//   flat_set(flat_set&&);
//   flat_set(std::vector<Key>, FlatContainerDupes);  // Re-use storage.
//   flat_set(std::initializer_list<value_type> ilist, FlatContainerDupes,
//            const Compare& comp = Compare());
//
// Assignment functions:
//   flat_set& operator=(const flat_set&);
//   flat_set& operator=(flat_set&&);
//   flat_set& operator=(initializer_list<Key>);
//
// Memory management functions:
//   void   reserve(size_t);
//   size_t capacity() const;
//   void   shrink_to_fit();
//
// Size management functions:
//   void   clear();
//   size_t size() const;
//   size_t max_size() const;
//   bool   empty() const;
//
// Iterator functions:
//   iterator               begin();
//   const_iterator         begin() const;
//   const_iterator         cbegin() const;
//   iterator               end();
//   const_iterator         end() const;
//   const_iterator         cend() const;
//   reverse_iterator       rbegin();
//   const reverse_iterator rbegin() const;
//   const_reverse_iterator crbegin() const;
//   reverse_iterator       rend();
//   const_reverse_iterator rend() const;
//   const_reverse_iterator crend() const;
//
// Insert and accessor functions:
//   pair<iterator, bool> insert(const Key&);
//   pair<iterator, bool> insert(Key&&);
//   void                 insert(InputIterator first, InputIterator last,
//                               FlatContainerDupes);
//   pair<iterator, bool> emplace(Args&&...);
//   iterator             emplace_hint(const_iterator, Args&&...);
//
// Erase functions:
//   iterator erase(const_iterator);
//   iterator erase(const_iterator first, const_iterator& last);
//   size_t   erase(const Key& key)
//
// Comparators (see std::set documentation).
//   key_compare   key_comp() const;
//   value_compare value_comp() const;
//
// Search functions:
//   size_t                   count(const Key&) const;
//   iterator                 find(const Key&);
//   const_iterator           find(const Key&) const;
//   pair<iterator, iterator> equal_range(Key&)
//   iterator                 lower_bound(const Key&);
//   const_iterator           lower_bound(const Key&) const;
//   iterator                 upper_bound(const Key&);
//   const_iterator           upper_bound(const Key&) const;
//
// General functions:
//   void swap(flat_set&&)
//
// Non-member operators:
//   bool operator==(const flat_set&, const flat_set);
//   bool operator!=(const flat_set&, const flat_set);
//   bool operator<(const flat_set&, const flat_set);
//   bool operator>(const flat_set&, const flat_set);
//   bool operator>=(const flat_set&, const flat_set);
//   bool operator<=(const flat_set&, const flat_set);
//
template <class Key, class Compare = std::less<Key>>
using flat_set = typename ::cr::internal::flat_tree<
    Key,
    Key,
    ::cr::internal::GetKeyFromValueIdentity<Key>,
    Compare>;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_FLAT_SET_H_
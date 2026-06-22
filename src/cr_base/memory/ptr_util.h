// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_MEMORY_PTR_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_MEMORY_PTR_UTIL_H_

#include <memory>
#include <utility>

#include "cr_base/base_export.h"

namespace cr {

// Helper to transfer ownership of a raw pointer to a std::unique_ptr<T>.
// Note that std::unique_ptr<T> has very different semantics from
// std::unique_ptr<T[]>: do not use this helper for array allocations.
template <typename T>
std::unique_ptr<T> WrapUnique(T* ptr) {
  return std::unique_ptr<T>(ptr);
}

// Function object which invokes 'free' on its parameter, which must be
// a pointer. Can be used to store malloc-allocated pointers in std::unique_ptr:
//
// std::unique_ptr<int, cr::FreeDeleter> foo_ptr(
//     static_cast<int*>(malloc(sizeof(int))));
struct FreeDeleter {
  inline void operator()(void* ptr) const {
    free(ptr);
  }
};

// TODO(crbug.com/817982): What we really need is for checked_math.h to be
// able to do checked arithmetic on pointers.
template <typename T>
inline uintptr_t AsUIntPtr(const T* t) {
  return reinterpret_cast<uintptr_t>(t);
}

// Fill memory with zeros in a way that the compiler doesn't optimize it away
// even if the pointer is not used afterwards.
CRBASE_EXPORT void ExplicitZeroMemory(void* ptr, size_t len);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MEMORY_PTR_UTIL_H_
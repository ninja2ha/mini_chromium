// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MEMORY_ALIGNED_MEMORY_H_
#define MINI_CHROMIUM_SRC_CRBASE_MEMORY_ALIGNED_MEMORY_H_

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

#include "crbase/base_export.h"
#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#include <malloc.h>
#else
#include <stdlib.h>
#endif

// A runtime sized aligned allocation can be created:
//
//   float* my_array = static_cast<float*>(AlignedAlloc(size, alignment));
//
//   // ... later, to release the memory:
//   AlignedFree(my_array);
//
// Or using unique_ptr:
//
//   std::unique_ptr<float, AlignedFreeDeleter> my_array(
//       static_cast<float*>(AlignedAlloc(size, alignment)));

namespace cr {

// This can be replaced with std::aligned_alloc when we have C++17.
// Caveat: std::aligned_alloc requires the size parameter be an integral
// multiple of alignment.
CRBASE_EXPORT void* AlignedAlloc(size_t size, size_t alignment);

inline void AlignedFree(void* ptr) {
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

// Deleter for use with unique_ptr. E.g., use as
//   std::unique_ptr<Foo, base::AlignedFreeDeleter> foo;
struct AlignedFreeDeleter {
  inline void operator()(void* ptr) const {
    AlignedFree(ptr);
  }
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MEMORY_ALIGNED_MEMORY_H_
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/memory/aligned_memory.h"

#include "crbase/logging.h"
#include "crbase/build_platform.h"

namespace cr {

void* AlignedAlloc(size_t size, size_t alignment) {
  CR_DCHECK(size > 0U);
  CR_DCHECK((alignment & (alignment - 1)) == 0U);
  CR_DCHECK((alignment % sizeof(void*)) == 0U);
  void* ptr = nullptr;
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
  ptr = _aligned_malloc(size, alignment);
#else
  if (int ret = posix_memalign(&ptr, alignment, size)) {
    CR_DLOG(Error) << "posix_memalign() returned with error " << ret;
    ptr = nullptr;
  }
#endif
  // Since aligned allocations may fail for non-memory related reasons, force a
  // crash if we encounter a failed allocation; maintaining consistent behavior
  // with a normal allocation failure in Chrome.
  if (!ptr) {
    CR_DLOG(Error) 
        << "If you crashed here, your aligned allocation is incorrect: "
        << "size=" << size << ", alignment=" << alignment;
    CR_CHECK(false);
  }
  // Sanity check alignment just to be safe.
  CR_DCHECK((reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0U);
  return ptr;
}

}  // namespace cr
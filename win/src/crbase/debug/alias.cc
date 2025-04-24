// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/debug/alias.h"
#include "crbase/build_platform.h"

namespace cr {
namespace debug {

// This file/function should be excluded from LTO/LTCG to ensure that the
// compiler can't see this function's implementation when compiling calls to it.
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
__declspec(noinline)
#elif defined(MINI_CHROMIUM_COMPILER_GCC)
__attribute__((noinline))
#endif
void Alias(const void* var) {}


}  // namespace debug
}  // namespace cr
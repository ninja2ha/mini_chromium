// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/debug/alias.h"
#include "crbuild/compiler_specific.h"

namespace cr {
namespace debug {

// This file/function should be excluded from LTO/LTCG to ensure that the
// compiler can't see this function's implementation when compiling calls to it.
CR_NOINLINE void Alias(const void* var) {}

}  // namespace debug
}  // namespace cr
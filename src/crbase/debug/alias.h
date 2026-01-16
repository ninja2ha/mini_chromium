// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_UTILS_DEBUG_ALIAS_H_
#define MINI_CHROMIUM_SRC_CRBASE_UTILS_DEBUG_ALIAS_H_

#include "crbase/base_export.h"

namespace cr {
namespace debug {

// Make the optimizer think that var is aliased. This is to prevent it from
// optimizing out local variables that would not otherwise be live at the point
// of a potential crash.
// cr::debug::Alias should only be used for local variables, not globals,
// object members, or function return values - these must be copied to locals if
// you want to ensure they are recorded in crash dumps.
// Note that if the local variable is a pointer then its value will be retained
// but the memory that it points to will probably not be saved in the crash
// dump - by default only stack memory is saved. Therefore the aliasing
// technique is usually only worthwhile with non-pointer variables. If you have
// a pointer to an object and you want to retain the object's state you need to
// copy the object or its fields to local variables. Example usage:
//   int last_error = err_;
//   cr::debug::Alias(&last_error);
//   CR_CHECK(false);
void CRBASE_EXPORT Alias(const void* var);

}  // namespace debug
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_UTILS_DEBUG_ALIAS_H_
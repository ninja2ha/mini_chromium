// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/debug/immediate_crash.h"

#include "crbase/debug/alias.h"

namespace cr {
namespace debug {

void ImmediateCrashBecauseOutofMemory(size_t size) {
  size_t tmp_size = size;
  cr::debug::Alias(&tmp_size);

  // Note: Don't add anything that may allocate here. Depending on the
  // allocator, this may be called from within the allocator (e.g. with
  // PartitionAlloc), and would deadlock as our locks are not recursive.
  //
  // Additionally, this is unlikely to work, since allocating from an OOM
  // handler is likely to fail.
  //
  // Use ImmediateCrash() so that the top frame in the crash is our code,
  // rather than using abort() or similar; this avoids the crash server needing
  // to be able to successfully unwind through libc to get to the correct
  // address, which is particularly an issue on Android.
  ImmediateCrash();
}

}  // namespace debug
}  // namespace cr
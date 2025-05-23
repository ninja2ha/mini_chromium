/*
 *  Copyright 2017 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "crbase/memory/zero_memory.h"

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#else
#include <string.h>
#endif

#include "crbase/logging.h"

namespace cr {

// Code and comment taken from "OPENSSL_cleanse" of BoringSSL.
void ExplicitZeroMemory(void* ptr, size_t len) {
  CR_DCHECK(ptr || !len);
#if defined(MINI_CHROMIUM_OS_WIN)
  SecureZeroMemory(ptr, len);
#else
  memset(ptr, 0, len);
#if !defined(__pnacl__)
  /* As best as we can tell, this is sufficient to break any optimisations that
     might try to eliminate "superfluous" memsets. If there's an easy way to
     detect memset_s, it would be better to use that. */
  __asm__ __volatile__("" : : "r"(ptr) : "memory");  // NOLINT
#endif
#endif  // !MINI_CHROMIUM_OS_WIN
}

}  // namespace cr

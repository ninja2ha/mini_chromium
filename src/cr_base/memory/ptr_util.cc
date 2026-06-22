// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/memory/ptr_util.h"

#include "cr_base/compiler_config.h"
#include "cr_base/logging/logging.h"

#if defined(MINI_CHROMIUM_OS_WIN)
// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>
#else
#include <string.h>
#endif

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
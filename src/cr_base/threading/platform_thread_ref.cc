// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/threading/platform_thread_ref.h"

#include <stddef.h>

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>
#else defined(MINI_CHROMIUM_OS_POSIX)
#include <pthread.h>
#endif

namespace cr {

// static 
PlatformThreadRef PlatformThreadRef::Current() {
#if defined(MINI_CHROMIUM_OS_WIN)
  return PlatformThreadRef(::GetCurrentThreadId());
#elif defined(MINI_CHROMIUM_OS_POSIX)
  return PlatformThreadRef(::pthread_self());
#else
#error Unsupported platform
#endif
}

}  // namespace cr
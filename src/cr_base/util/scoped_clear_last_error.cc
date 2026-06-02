// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/util/scoped_clear_last_error.h"

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>

namespace cr {

ScopedClearLastError::ScopedClearLastError()
    : ScopedClearLastErrorBase(), last_system_error_(::GetLastError()) {
  ::SetLastError(0);
}

ScopedClearLastError::~ScopedClearLastError() {
  ::SetLastError(last_system_error_);
}

}  // namespace cr

#endif  // defined(MINI_CHROMIUM_OS_WIN)
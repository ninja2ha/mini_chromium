// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/win/scoped_handle.h"

// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>

namespace cr {
namespace win {

// Static.
bool HandleTraits::CloseHandle(HandleTraits::Handle handle) {
  return !!::CloseHandle(handle);
}

}  // namespace win
}  // namespace cr
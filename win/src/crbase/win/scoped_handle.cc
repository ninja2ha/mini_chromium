// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/win/scoped_handle.h"

#include <windows.h>

namespace cr {
namespace win {

// Static.
bool HandleTraits::CloseHandle(HandleTraits::Handle handle) {
  return !!::CloseHandle(handle);
}

}  // namespace win
}  // namespace cr
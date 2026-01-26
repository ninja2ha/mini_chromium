// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/helper/scoped_clear_last_error.h"

#include "crbase/win/windows_types.h"

namespace cr {

ScopedClearLastError::ScopedClearLastError()
    : ScopedClearLastErrorBase(), last_system_error_(GetLastError()) {
  SetLastError(0);
}

ScopedClearLastError::~ScopedClearLastError() {
  SetLastError(last_system_error_);
}

}  // namespace cr

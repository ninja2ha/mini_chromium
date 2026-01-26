// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_INTERNAL_WIN_H_
#define MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_INTERNAL_WIN_H_

#include <string>

#include "crbase/logging/logging_types.h"

namespace cr {
namespace logging {
namespace internal {

// Translating error code to system message.
std::string strerror(SystemErrorCode error_code) ;

}  // namespace internal
}  // namespace logging
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_INTERNAL_WIN_H_
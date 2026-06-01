// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_STRERROR_H_
#define MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_STRERROR_H_

#include <stdint.h>

#include <string>

#include "cr_base/compiler_config.h"

#include "cr_base/base_export.h"
#include "cr_base/logging/logging_types.h"

namespace cr {
namespace logging {

// Translating error code to system message, encoding with utf-8.
CRBASE_EXPORT std::string StrError(SystemErrorCode error_code) ;

#if defined(MINI_CHROMIUM_OS_WIN)
// Translating error code to system message, encoding with unicode-16, 
// used for system debug logging(API: OutputDebugStringW).
CRBASE_EXPORT std::wstring StrErrorW(SystemErrorCode error_code);

#elif defined(MINI_CHROMIUM_OS_POSIX)
// BEFORE using anything from this file, first look at CR_PLOG and friends in
// logging.h and use them instead if applicable.
//
// This file declares safe, portable alternatives to the POSIX strerror()
// function. strerror() is inherently unsafe in multi-threaded apps and should
// never be used. Doing so can cause crashes. Additionally, the thread-safe
// alternative strerror_r varies in semantics across platforms. Use these
// functions instead.

// Thread-safe strerror function with dependable semantics that never fails.
// It will write the string form of error "err" to buffer buf of length len.
// If there is an error calling the OS's strerror_r() function then a message to
// that effect will be printed into buf, truncating if necessary. The final
// result is always null-terminated. The value of errno is never changed.
//
// Use this instead of strerror_r().
CRBASE_EXPORT void SafeStrError(SystemErrorCode error_code,
                                char* buf,
                                size_t buf_len);
#endif

}  // namespace logging
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_STRERROR_H_
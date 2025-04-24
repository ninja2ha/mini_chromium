// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_HELPER_SCOPED_CLEAR_LAST_ERROR_H_
#define MINI_CHROMIUM_SRC_CRBASE_HELPER_SCOPED_CLEAR_LAST_ERROR_H_

#include <errno.h>

#include "crbase/base_export.h"
#include "crbase/build_platform.h"

namespace cr {
namespace internal {

// ScopedClearLastError stores and resets the value of thread local error codes
// (errno, GetLastError()), and restores them in the destructor. This is useful
// to avoid side effects on these values in instrumentation functions that
// interact with the OS.

// Common implementation of ScopedClearLastError for all platforms. Use
// ScopedClearLastError instead.
class CRBASE_EXPORT ScopedClearLastErrorBase {
 public:
  ScopedClearLastErrorBase(
      const ScopedClearLastErrorBase&) = delete;
  ScopedClearLastErrorBase& operator=(
      const ScopedClearLastErrorBase&) = delete;

  ScopedClearLastErrorBase() : last_errno_(errno) { errno = 0; }
  ~ScopedClearLastErrorBase() { errno = last_errno_; }

 private:
  const int last_errno_;
};

#if defined(MINI_CHROMIUM_OS_WIN)

// Windows specific implementation of ScopedClearLastError.
class CRBASE_EXPORT ScopedClearLastError : public ScopedClearLastErrorBase {
 public:
  ScopedClearLastError(const ScopedClearLastError&) = delete;
  ScopedClearLastError& operator=(const ScopedClearLastError&) = delete;

  ScopedClearLastError();
  ~ScopedClearLastError();

 private:
  unsigned int last_system_error_;
};

#elif defined(MINI_CHROMIUM_OS_POSIX)

using ScopedClearLastError = ScopedClearLastErrorBase;

#endif  // defined(MINI_CHROMIUM_OS_WIN)

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_HELPER_SCOPED_CLEAR_LAST_ERROR_H_
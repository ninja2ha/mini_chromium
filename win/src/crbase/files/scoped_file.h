// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_FILES_SCOPED_FILE_H_
#define MINI_CHROMIUM_SRC_CRBASE_FILES_SCOPED_FILE_H_

#include <stdio.h>

#include <memory>

#include "crbase/base_export.h"
#include "crbase/logging.h"
#include "crbase/helper/scoped_generic.h"
#include "crbase/build_platform.h"

namespace crbase {

namespace internal {

#if defined(MINI_CHROMIUM_OS_POSIX)
struct CRBASE_EXPORT ScopedFDCloseTraits {
  static int InvalidValue() {
    return -1;
  }
  static void Free(int fd);
};
#endif

// Functor for |ScopedFILE| (below).
struct ScopedFILECloser {
  inline void operator()(FILE* x) const {
    if (x)
      fclose(x);
  }
};

}  // namespace internal

// -----------------------------------------------------------------------------

#if defined(MINI_CHROMIUM_OS_POSIX)
// A low-level Posix file descriptor closer class. Use this when writing
// platform-specific code, especially that does non-file-like things with the
// FD (like sockets).
//
// If you're writing low-level Windows code, see base/win/scoped_handle.h
// which provides some additional functionality.
//
// If you're writing cross-platform code that deals with actual files, you
// should generally use base::File instead which can be constructed with a
// handle, and in addition to handling ownership, has convenient cross-platform
// file manipulation functions on it.
typedef ScopedGeneric<int, internal::ScopedFDCloseTraits> ScopedFD;
#endif

// Automatically closes |FILE*|s.
typedef std::unique_ptr<FILE, internal::ScopedFILECloser> ScopedFILE;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_FILES_SCOPED_FILE_H_
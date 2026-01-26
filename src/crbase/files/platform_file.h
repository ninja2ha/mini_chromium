// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_FILES_PLATFORM_FILE_H_
#define MINI_CHROMIUM_SRC_CRBASE_FILES_PLATFORM_FILE_H_

#include "crbase/files/scoped_file.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/scoped_handle.h"
#include "crbase/win/windows_types.h"
#endif

// This file defines platform-independent types for dealing with
// platform-dependent files. If possible, use the higher-level cr::File class
// rather than these primitives.

namespace cr {

#if defined(MINI_CHROMIUM_OS_WIN)

using PlatformFile = HANDLE;
using ScopedPlatformFile = ::cr::win::ScopedHandle;

// It would be nice to make this constexpr but INVALID_HANDLE_VALUE is a
// ((void*)(-1)) which Clang rejects since reinterpret_cast is technically
// disallowed in constexpr. Visual Studio accepts this, however.
const PlatformFile kInvalidPlatformFile = INVALID_HANDLE_VALUE;

#elif defined(MINI_CHROMIUM_OS_POSIX)

using PlatformFile = int;
using ScopedPlatformFile = ::cr::ScopedFD;

constexpr PlatformFile kInvalidPlatformFile = -1;

#endif

}  // namespace

#endif  // MINI_CHROMIUM_SRC_CRBASE_FILES_PLATFORM_FILE_H_

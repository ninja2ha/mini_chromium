// Copyright 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_WIN_API_HELPER_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_WIN_API_HELPER_H_

#include "crbase/base_export.h"
#include "crbase/win/windows_types.h"

namespace cr {
namespace win {

CRBASE_EXPORT HMODULE GetNtDllModule();
CRBASE_EXPORT HMODULE GetKernel32Module();
CRBASE_EXPORT HMODULE GetKernelBaseModule();

typedef LPWSTR(WINAPI* CharUpperWFunction)(LPWSTR lpsz);
CRBASE_EXPORT CharUpperWFunction GetKernelBaseCharUpperW();

} // namespace win
} // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_WIN_API_HELPER_H_

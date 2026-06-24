// Copyright 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/win/win_util.h"

typedef struct IUnknown IUnknown;
#include <windows.h>

#include "cr_base/memory/no_destructor.h"

namespace cr {
namespace win {

HMODULE GetNtDllModule() {
  static cr::NoDestructor<HMODULE> ntdll([] {
    return GetModuleHandleW(L"ntdll.dll");
  }());
  return *ntdll;
}

HMODULE GetKernel32Module() {
  static cr::NoDestructor<HMODULE> kernel32([]{
    return GetModuleHandleW(L"kernel32.dll");
  }());
  return *kernel32;
}

HMODULE GetKernelBaseModule() {
  static cr::NoDestructor<HMODULE> kernelbase([] {
    return GetModuleHandleW(L"kernelbase.dll");
  }());
  return *kernelbase;
}

} // namespace win
} // namespace cr

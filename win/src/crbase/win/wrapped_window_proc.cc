// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/win/wrapped_window_proc.h"

#include "crbase/atomic/atomicops.h"
#include "crbase/logging.h"
#include "crbase/strings/string_util.h"

namespace {

cr::win::WinProcExceptionFilter s_exception_filter = nullptr;

HMODULE GetModuleFromWndProc(WNDPROC window_proc) {
  HMODULE instance = nullptr;
  // Converting a pointer-to-function to a void* is undefined behavior, but
  // Windows (and POSIX) APIs require it to work.
  void* address = reinterpret_cast<void*>(window_proc);
  if (!::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            static_cast<char*>(address), &instance)) {
    CR_NOTREACHED();
  }
  return instance;
}

}  // namespace.

namespace cr {
namespace win {

WinProcExceptionFilter SetWinProcExceptionFilter(
    WinProcExceptionFilter filter) {
  subtle::AtomicWord rv = subtle::NoBarrier_AtomicExchange(
      reinterpret_cast<subtle::AtomicWord*>(&s_exception_filter),
      reinterpret_cast<subtle::AtomicWord>(filter));
  return reinterpret_cast<WinProcExceptionFilter>(rv);
}

int CallExceptionFilter(EXCEPTION_POINTERS* info) {
  return s_exception_filter ? s_exception_filter(info)
                            : EXCEPTION_CONTINUE_SEARCH;
}

void InitializeWindowClass(const char16* class_name,
                           WNDPROC window_proc,
                           UINT style,
                           int class_extra,
                           int window_extra,
                           HCURSOR cursor,
                           HBRUSH background,
                           const char16* menu_name,
                           HICON large_icon,
                           HICON small_icon,
                           WNDCLASSEXW* class_out) {
  class_out->cbSize = sizeof(WNDCLASSEXW);
  class_out->style = style;
  class_out->lpfnWndProc = window_proc;
  class_out->cbClsExtra = class_extra;
  class_out->cbWndExtra = window_extra;
  // RegisterClassEx uses a handle of the module containing the window procedure
  // to distinguish identically named classes registered in different modules.
  class_out->hInstance = GetModuleFromWndProc(window_proc);
  class_out->hIcon = large_icon;
  class_out->hCursor = cursor;
  class_out->hbrBackground = background;
  class_out->lpszMenuName = menu_name;
  class_out->lpszClassName = class_name;
  class_out->hIconSm = small_icon;

  // Check if |window_proc| is valid.
  CR_DCHECK(class_out->hInstance != nullptr);
}

}  // namespace win
}  // namespace cr
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Provides a way to handle exceptions that happen while a WindowProc is
// running. The behavior of exceptions generated inside a WindowProc is OS
// dependent, but it is possible that the OS just ignores the exception and
// continues execution, which leads to unpredictable behavior for Chrome.

#ifndef MINI_CHROMIUM_CRBASE_WIN_WRAPPED_WINDOW_PROC_H_
#define MINI_CHROMIUM_CRBASE_WIN_WRAPPED_WINDOW_PROC_H_

#include <windows.h>

#include "crbase/base_export.h"
#include "crbase/strings/string16.h"

namespace cr {
namespace win {

// An exception filter for a WindowProc. The return value determines how the
// exception should be handled, following standard SEH rules. However, the
// expected behavior for this function is to not return, instead of returning
// EXCEPTION_EXECUTE_HANDLER or similar, given that in general we are not
// prepared to handle exceptions.
typedef int (__cdecl* WinProcExceptionFilter)(EXCEPTION_POINTERS* info);

// Sets the filter to deal with exceptions inside a WindowProc. Returns the old
// exception filter, if any.
// This function should be called before any window is created.
CRBASE_EXPORT WinProcExceptionFilter
SetWinProcExceptionFilter(WinProcExceptionFilter filter);

// Calls the registered exception filter.
CRBASE_EXPORT int CallExceptionFilter(EXCEPTION_POINTERS* info);

// Initializes the WNDCLASSEX structure |*class_out| to be passed to
// RegisterClassEx() making sure that it is associated with the module
// containing the window procedure.
CRBASE_EXPORT void InitializeWindowClass(const char16* class_name,
                                         WNDPROC window_proc,
                                         UINT style,
                                         int class_extra,
                                         int window_extra,
                                         HCURSOR cursor,
                                         HBRUSH background,
                                         const char16* menu_name,
                                         HICON large_icon,
                                         HICON small_icon,
                                         WNDCLASSEXW* class_out);

// Wrapper that supplies a standard exception frame for the provided WindowProc.
// The normal usage is something like this:
//
// LRESULT CALLBACK MyWinProc(HWND hwnd, UINT message,
//                            WPARAM wparam, LPARAM lparam) {
//   // Do Something.
// }
//
// ...
//
//   WNDCLASSEX wc = {0};
//   wc.lpfnWndProc = WrappedWindowProc<MyWinProc>;
//   wc.lpszClassName = class_name;
//   ...
//   RegisterClassEx(&wc);
//
//   CreateWindowW(class_name, window_name, ...
//
template <WNDPROC proc>
LRESULT CALLBACK
WrappedWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  LRESULT rv = 0;
  __try {
    rv = proc(hwnd, message, wparam, lparam);
  } __except (CallExceptionFilter(GetExceptionInformation())) {
  }
  return rv;
}

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_CRBASE_WIN_WRAPPED_WINDOW_PROC_H_
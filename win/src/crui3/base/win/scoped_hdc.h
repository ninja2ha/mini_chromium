// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_WIN_SCOPED_HDC_H_
#define BASE_WIN_SCOPED_HDC_H_

#include <windows.h>

///#include "crbase/debug/gdi_debug_util_win.h"
#include "crbase/logging.h"
#include "crbase/win/scoped_handle.h"

namespace crui {
namespace win {

// Like ScopedHandle but for HDC.  Only use this on HDCs returned from
// GetDC.
class ScopedGetDC {
 public:
  ScopedGetDC(const ScopedGetDC&) = delete;
  ScopedGetDC& operator=(const ScopedGetDC&) = delete;

  explicit ScopedGetDC(HWND hwnd) : hwnd_(hwnd), hdc_(GetDC(hwnd)) {
    if (hwnd_) {
      CR_DCHECK(IsWindow(hwnd_));
      CR_DCHECK(hdc_);
    } else {
      // If GetDC(NULL) returns NULL, something really bad has happened, like
      // GDI handle exhaustion.  In this case Chrome is going to behave badly no
      // matter what, so we may as well just force a crash now.
      if (!hdc_)
        CR_CHECK(false);
      ///  base::debug::CollectGDIUsageAndDie();
    }
  }

  ~ScopedGetDC() {
    if (hdc_)
      ReleaseDC(hwnd_, hdc_);
  }

  operator HDC() { return hdc_; }

 private:
  HWND hwnd_;
  HDC hdc_;
};

// Like ScopedHandle but for HDC.  Only use this on HDCs returned from
// CreateCompatibleDC, CreateDC and CreateIC.
class CreateDCTraits {
 public:
  typedef HDC Handle;

  CreateDCTraits() = delete;
  CreateDCTraits(const CreateDCTraits&) = delete;
  CreateDCTraits& operator=(const CreateDCTraits&) = delete;

  static bool CloseHandle(HDC handle) { return ::DeleteDC(handle) != FALSE; }

  static bool IsHandleValid(HDC handle) { return handle != NULL; }

  static HDC NullHandle() { return NULL; }
};

typedef cr::win::GenericScopedHandle<
    CreateDCTraits, 
    cr::win::DummyVerifierTraits> ScopedCreateDC;

}  // namespace win
}  // namespace crui

#endif  // BASE_WIN_SCOPED_HDC_H_

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_SCOPED_OLE_INITIALIZER_H_
#define UI_BASE_WIN_SCOPED_OLE_INITIALIZER_H_

#include <windows.h>

#include "crbase/threading/thread_checker.h"
#include "crui/base/ui_export.h"

namespace crui {

class CRUI_EXPORT ScopedOleInitializer {
 public:
  ScopedOleInitializer(const ScopedOleInitializer&) = delete;
  ScopedOleInitializer& operator=(const ScopedOleInitializer&) = delete;

  ScopedOleInitializer();
  ~ScopedOleInitializer();

 private:
  HRESULT hr_;
  CR_THREAD_CHECKER(thread_checker_)
};

}  // namespace crui

#endif  // UI_BASE_WIN_SCOPED_OLE_INITIALIZER_H_

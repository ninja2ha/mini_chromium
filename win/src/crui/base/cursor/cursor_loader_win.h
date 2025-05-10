// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CURSOR_CURSOR_LOADER_WIN_H_
#define UI_BASE_CURSOR_CURSOR_LOADER_WIN_H_

#include "crbase/compiler_specific.h"
#include "crui/base/cursor/cursor_loader.h"

namespace crui {

class CRUI_EXPORT CursorLoaderWin : public CursorLoader {
 public:
  CursorLoaderWin(const CursorLoaderWin&) = delete;
  CursorLoaderWin& operator=(const CursorLoaderWin&) = delete;

  CursorLoaderWin();
  ~CursorLoaderWin() override;

  // Overridden from CursorLoader:
  void LoadImageCursor(CursorType id,
                       int resource_id,
                       const gfx::Point& hot) override;
  void LoadAnimatedCursor(CursorType id,
                          int resource_id,
                          const gfx::Point& hot,
                          int frame_delay_ms) override;
  void UnloadAll() override;
  void SetPlatformCursor(gfx::NativeCursor* cursor) override;

  // Used to pass the cursor resource module name to the cursor loader. This is
  // typically used to load non system cursors.
  static void SetCursorResourceModule(const cr::string16& module_name);
};

}  // namespace crui

#endif  // UI_BASE_CURSOR_CURSOR_LOADER_WIN_H_

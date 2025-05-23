// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_NATIVE_CURSOR_MANAGER_DELEGATE_H_
#define UI_WM_CORE_NATIVE_CURSOR_MANAGER_DELEGATE_H_

#include "crui/base/cursor/cursor.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace wm {

// The non-public interface that CursorManager exposes to its users. This
// gives accessors to all the current state, and mutators to all the current
// state.
class CRUI_EXPORT NativeCursorManagerDelegate {
 public:
  virtual ~NativeCursorManagerDelegate() {}

  // TODO(tdanderson): Possibly remove this interface.
  virtual gfx::NativeCursor GetCursor() const = 0;
  virtual bool IsCursorVisible() const = 0;

  virtual void CommitCursor(gfx::NativeCursor cursor) = 0;
  virtual void CommitVisibility(bool visible) = 0;
  virtual void CommitCursorSize(crui::CursorSize cursor_size) = 0;
  virtual void CommitMouseEventsEnabled(bool enabled) = 0;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_NATIVE_CURSOR_MANAGER_DELEGATE_H_

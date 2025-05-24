// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_CURSOR_CLIENT_OBSERVER_H_
#define UI_AURA_CLIENT_CURSOR_CLIENT_OBSERVER_H_

#include "crui/base/ui_export.h"
#include "crui/base/cursor/cursor.h"

namespace crui {

namespace display {
class Display;
}

namespace aura {
namespace client {

class CRUI_EXPORT CursorClientObserver {
 public:
  virtual void OnCursorVisibilityChanged(bool is_visible) {}
  virtual void OnCursorSizeChanged(crui::CursorSize cursor_size) {}
  virtual void OnCursorDisplayChanged(const display::Display& display) {}

 protected:
  virtual ~CursorClientObserver() {}
};

}  // namespace client
}  // namespace aura

}  // namespace crui

#endif  // UI_AURA_CLIENT_CURSOR_CLIENT_OBSERVER_H_

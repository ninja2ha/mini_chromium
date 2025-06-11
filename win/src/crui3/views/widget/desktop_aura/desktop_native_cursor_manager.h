// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_NATIVE_CURSOR_MANAGER_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_NATIVE_CURSOR_MANAGER_H_

#include <memory>
#include <set>

#include "crbase/compiler_specific.h"
#include "crui/aura/wm/core/native_cursor_manager.h"
#include "crui/base/ui_export.h"

namespace crui {
class CursorLoader;

namespace aura {
class WindowTreeHost;
}  // namespace aura

namespace wm {
class NativeCursorManagerDelegate;
}  // namespace wm

namespace views {

// A NativeCursorManager that performs the desktop-specific setting of cursor
// state. Similar to NativeCursorManagerAsh, it also communicates these changes
// to all root windows.
class CRUI_EXPORT DesktopNativeCursorManager
    : public wm::NativeCursorManager {
 public:
  DesktopNativeCursorManager(const DesktopNativeCursorManager&) = delete;
  DesktopNativeCursorManager& operator=(
      const DesktopNativeCursorManager&) = delete;

  DesktopNativeCursorManager();
  ~DesktopNativeCursorManager() override;

  // Builds a cursor and sets the internal platform representation. The return
  // value should not be cached.
  gfx::NativeCursor GetInitializedCursor(crui::CursorType type);

  // Adds |host| to the set |hosts_|.
  void AddHost(aura::WindowTreeHost* host);

  // Removes |host| from the set |hosts_|.
  void RemoveHost(aura::WindowTreeHost* host);

 private:
  // Overridden from wm::NativeCursorManager:
  void SetDisplay(const display::Display& display,
                  wm::NativeCursorManagerDelegate* delegate) override;
  void SetCursor(gfx::NativeCursor cursor,
                 wm::NativeCursorManagerDelegate* delegate) override;
  void SetVisibility(bool visible,
                     wm::NativeCursorManagerDelegate* delegate) override;
  void SetCursorSize(crui::CursorSize cursor_size,
                     wm::NativeCursorManagerDelegate* delegate) override;
  void SetMouseEventsEnabled(
      bool enabled,
      wm::NativeCursorManagerDelegate* delegate) override;

  // The set of hosts to notify of changes in cursor state.
  using Hosts = std::set<aura::WindowTreeHost*>;
  Hosts hosts_;

  std::unique_ptr<crui::CursorLoader> cursor_loader_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_NATIVE_CURSOR_MANAGER_H_

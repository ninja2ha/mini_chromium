// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_native_cursor_manager.h"

#include <utility>

#include "crui/aura/window_event_dispatcher.h"
#include "crui/aura/window_tree_host.h"
#include "crui/base/cursor/cursor_loader.h"

namespace crui {
namespace views {

DesktopNativeCursorManager::DesktopNativeCursorManager()
    : cursor_loader_(crui::CursorLoader::Create()) {}

DesktopNativeCursorManager::~DesktopNativeCursorManager() = default;

gfx::NativeCursor DesktopNativeCursorManager::GetInitializedCursor(
    crui::CursorType type) {
  gfx::NativeCursor cursor(type);
  cursor_loader_->SetPlatformCursor(&cursor);
  return cursor;
}

void DesktopNativeCursorManager::AddHost(aura::WindowTreeHost* host) {
  hosts_.insert(host);
}

void DesktopNativeCursorManager::RemoveHost(aura::WindowTreeHost* host) {
  hosts_.erase(host);
}

void DesktopNativeCursorManager::SetDisplay(
    const display::Display& display,
    wm::NativeCursorManagerDelegate* delegate) {
  cursor_loader_->UnloadAll();
  cursor_loader_->set_rotation(display.rotation());
  cursor_loader_->set_scale(display.device_scale_factor());

  SetCursor(delegate->GetCursor(), delegate);
}

void DesktopNativeCursorManager::SetCursor(
    gfx::NativeCursor cursor,
    wm::NativeCursorManagerDelegate* delegate) {
  gfx::NativeCursor new_cursor = cursor;
  cursor_loader_->SetPlatformCursor(&new_cursor);
  delegate->CommitCursor(new_cursor);

  if (delegate->IsCursorVisible()) {
    for (auto* host : hosts_)
      host->SetCursor(new_cursor);
  }
}

void DesktopNativeCursorManager::SetVisibility(
    bool visible,
    wm::NativeCursorManagerDelegate* delegate) {
  delegate->CommitVisibility(visible);

  if (visible) {
    SetCursor(delegate->GetCursor(), delegate);
  } else {
    gfx::NativeCursor invisible_cursor(crui::CursorType::kNone);
    cursor_loader_->SetPlatformCursor(&invisible_cursor);
    for (auto* host : hosts_)
      host->SetCursor(invisible_cursor);
  }

  for (auto* host : hosts_)
    host->OnCursorVisibilityChanged(visible);
}

void DesktopNativeCursorManager::SetCursorSize(
    crui::CursorSize cursor_size,
    wm::NativeCursorManagerDelegate* delegate) {
  CR_NOTIMPLEMENTED();
}

void DesktopNativeCursorManager::SetMouseEventsEnabled(
    bool enabled,
    wm::NativeCursorManagerDelegate* delegate) {
  delegate->CommitMouseEventsEnabled(enabled);

  // TODO(erg): In the ash version, we set the last mouse location on Env. I'm
  // not sure this concept makes sense on the desktop.

  SetVisibility(delegate->IsCursorVisible(), delegate);

  for (auto* host : hosts_)
    host->dispatcher()->OnMouseEventsEnableStateChanged(enabled);
}

}  // namespace views
}  // namespace crui

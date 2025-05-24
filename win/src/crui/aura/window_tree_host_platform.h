// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_WINDOW_TREE_HOST_PLATFORM_H_
#define UI_AURA_WINDOW_TREE_HOST_PLATFORM_H_

#include <memory>

#include "crbase/compiler_specific.h"
#include "crui/base/ui_export.h"
#include "crui/aura/client/window_types.h"
#include "crui/aura/window.h"
#include "crui/aura/window_tree_host.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/aura/platform_window/platform_window_delegate.h"

namespace crui {

enum class DomCode;
class PlatformWindow;
class KeyboardHook;
struct PlatformWindowInitProperties;

namespace aura {

// The unified WindowTreeHost implementation for platforms
// that implement PlatformWindow.
class CRUI_EXPORT WindowTreeHostPlatform : public WindowTreeHost,
                                           public crui::PlatformWindowDelegate {
 public:
  WindowTreeHostPlatform(const WindowTreeHostPlatform&) = delete;
  WindowTreeHostPlatform& operator=(const WindowTreeHostPlatform&) = delete;

  // See Compositor() for details on |trace_environment_name|.
  explicit WindowTreeHostPlatform(
      crui::PlatformWindowInitProperties properties,
      std::unique_ptr<Window> = nullptr,
      const char* trace_environment_name = nullptr,
      bool use_external_begin_frame_control = false);
  ~WindowTreeHostPlatform() override;

  // WindowTreeHost:
  crui::EventSource* GetEventSource() override;
  gfx::AcceleratedWidget GetAcceleratedWidget() override;
  void ShowImpl() override;
  void HideImpl() override;
  gfx::Rect GetBoundsInPixels() const override;
  void SetBoundsInPixels(const gfx::Rect& bounds) override;
  gfx::Point GetLocationOnScreenInPixels() const override;
  void SetCapture() override;
  void ReleaseCapture() override;
  void SetCursorNative(gfx::NativeCursor cursor) override;
  void MoveCursorToScreenLocationInPixels(
      const gfx::Point& location_in_pixels) override;
  void OnCursorVisibilityChangedNative(bool show) override;

 protected:
  // NOTE: this does not call CreateCompositor(); subclasses must call
  // CreateCompositor() at the appropriate time.
  explicit WindowTreeHostPlatform(std::unique_ptr<Window> window = nullptr);

  // Creates a ui::PlatformWindow appropriate for the current platform and
  // installs it at as the PlatformWindow for this WindowTreeHostPlatform.
  void CreateAndSetPlatformWindow(crui::PlatformWindowInitProperties properties);

  void SetPlatformWindow(std::unique_ptr<crui::PlatformWindow> window);
  crui::PlatformWindow* platform_window() { return platform_window_.get(); }
  const crui::PlatformWindow* platform_window() const {
    return platform_window_.get();
  }

  // ui::PlatformWindowDelegate:
  void OnBoundsChanged(const gfx::Rect& new_bounds) override;
  void OnDamageRect(const gfx::Rect& damaged_region) override;
  void DispatchEvent(crui::Event* event) override;
  void OnCloseRequest() override;
  void OnClosed() override;
  void OnWindowStateChanged(crui::PlatformWindowState new_state) override;
  void OnLostCapture() override;
  void OnAcceleratedWidgetAvailable(gfx::AcceleratedWidget widget) override;
  void OnAcceleratedWidgetDestroyed() override;
  void OnActivationChanged(bool active) override;
  void OnMouseEnter() override;

  // Overridden from aura::WindowTreeHost:
  bool CaptureSystemKeyEventsImpl(
      cr::Optional<cr::flat_set<crui::DomCode>> dom_codes) override;
  void ReleaseSystemKeyEventCapture() override;
  bool IsKeyLocked(crui::DomCode dom_code) override;
  cr::flat_map<std::string, std::string> GetKeyboardLayoutMap() override;

 private:
  gfx::AcceleratedWidget widget_;
  std::unique_ptr<crui::PlatformWindow> platform_window_;
  gfx::NativeCursor current_cursor_;
  gfx::Rect bounds_in_pixels_;

  std::unique_ptr<crui::KeyboardHook> keyboard_hook_;

  gfx::Size pending_size_;

  // Tracks how nested OnBoundsChanged() is. That is, on entering
  // OnBoundsChanged() this is incremented and on leaving OnBoundsChanged() this
  // is decremented.
  int on_bounds_changed_recursion_depth_ = 0;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_WINDOW_TREE_HOST_PLATFORM_H_

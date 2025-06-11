// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/window_tree_host_platform.h"

#include <utility>

#include "crbase/functional/bind.h"
#include "crbase/message_loop/run_loop.h"
///#include "base/trace_event/trace_event.h"
#include "crui/aura/client/cursor_client.h"
#include "crui/aura/env.h"
#include "crui/aura/window.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/aura/window_tree_host_observer.h"
#include "crui/base/layout.h"
///#include "crui/compositor/compositor.h"
#include "crui/display/display.h"
#include "crui/display/screen.h"
#include "crui/events/event.h"
#include "crui/events/keyboard_hook.h"
#include "crui/events/keycodes/dom/dom_code.h"
#include "crui/aura/platform_window/platform_window.h"
#include "crui/aura/platform_window/platform_window_init_properties.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crui/base/cursor/cursor_loader_win.h"
#include "crui/aura/platform_window/win/win_window.h"
#endif

#if defined(MINI_CHROMIUM_USE_X11)
#include "crui/aura/platform_window/x11/x11_window.h"  // nogncheck
#else
#include "crui/events/keycodes/dom/dom_keyboard_layout_map.h"
#endif

namespace crui {
namespace aura {

// static
std::unique_ptr<WindowTreeHost> WindowTreeHost::Create(
    crui::PlatformWindowInitProperties properties) {
  return std::make_unique<WindowTreeHostPlatform>(
      std::move(properties),
      std::make_unique<aura::Window>(nullptr, client::WINDOW_TYPE_UNKNOWN));
}

WindowTreeHostPlatform::WindowTreeHostPlatform(
    crui::PlatformWindowInitProperties properties,
    std::unique_ptr<Window> window,
    const char* trace_environment_name,
    bool use_external_begin_frame_control)
    : WindowTreeHost(std::move(window)) {
  bounds_in_pixels_ = properties.bounds;
  CreateCompositor(///viz::FrameSinkId(),
                   /* force_software_compositor */ false,
                   use_external_begin_frame_control, trace_environment_name);
  CreateAndSetPlatformWindow(std::move(properties));
}

WindowTreeHostPlatform::WindowTreeHostPlatform(std::unique_ptr<Window> window)
    : WindowTreeHost(std::move(window)),
      widget_(gfx::kNullAcceleratedWidget),
      current_cursor_(crui::CursorType::kNull) {}

void WindowTreeHostPlatform::CreateAndSetPlatformWindow(
    crui::PlatformWindowInitProperties properties) {
#if defined(MINI_CHROMIUM_OS_WIN)
  platform_window_.reset(new crui::WinWindow(this, properties.bounds));
#elif defined(MINI_CHROMIUM_USE_X11)
  auto x11_window = std::make_unique<crui::X11Window>(this);
  x11_window->Initialize(std::move(properties));
  SetPlatformWindow(std::move(x11_window));
#else
  CR_NOTIMPLEMENTED();
#endif
}

void WindowTreeHostPlatform::SetPlatformWindow(
    std::unique_ptr<crui::PlatformWindow> window) {
  platform_window_ = std::move(window);
}

WindowTreeHostPlatform::~WindowTreeHostPlatform() {
  DestroyCompositor();
  DestroyDispatcher();

  // |platform_window_| may not exist yet.
  if (platform_window_)
    platform_window_->Close();
}

crui::EventSource* WindowTreeHostPlatform::GetEventSource() {
  return this;
}

gfx::AcceleratedWidget WindowTreeHostPlatform::GetAcceleratedWidget() {
  return widget_;
}

void WindowTreeHostPlatform::ShowImpl() {
  platform_window_->Show();
}

void WindowTreeHostPlatform::HideImpl() {
  platform_window_->Hide();
}

gfx::Rect WindowTreeHostPlatform::GetBoundsInPixels() const {
  return platform_window_ ? platform_window_->GetBounds() : gfx::Rect();
}

void WindowTreeHostPlatform::SetBoundsInPixels(const gfx::Rect& bounds) {
  pending_size_ = bounds.size();
  platform_window_->SetBounds(bounds);
}

gfx::Point WindowTreeHostPlatform::GetLocationOnScreenInPixels() const {
  return platform_window_->GetBounds().origin();
}

void WindowTreeHostPlatform::SetCapture() {
  platform_window_->SetCapture();
}

void WindowTreeHostPlatform::ReleaseCapture() {
  platform_window_->ReleaseCapture();
}

bool WindowTreeHostPlatform::CaptureSystemKeyEventsImpl(
    cr::Optional<cr::flat_set<crui::DomCode>> dom_codes) {
  // Only one KeyboardHook should be active at a time, otherwise there will be
  // problems with event routing (i.e. which Hook takes precedence) and
  // destruction ordering.
  CR_DCHECK(!keyboard_hook_);
  keyboard_hook_ = crui::KeyboardHook::CreateModifierKeyboardHook(
      std::move(dom_codes), GetAcceleratedWidget(),
      cr::BindRepeating(
          [](crui::PlatformWindowDelegate* delegate, crui::KeyEvent* event) {
            delegate->DispatchEvent(event);
          },
          cr::Unretained(this)));

  return keyboard_hook_ != nullptr;
}

void WindowTreeHostPlatform::ReleaseSystemKeyEventCapture() {
  keyboard_hook_.reset();
}

bool WindowTreeHostPlatform::IsKeyLocked(crui::DomCode dom_code) {
  return keyboard_hook_ && keyboard_hook_->IsKeyLocked(dom_code);
}

cr::flat_map<std::string, std::string>
WindowTreeHostPlatform::GetKeyboardLayoutMap() {
#if !defined(MINI_CHROMIUM_USE_X11)
  return crui::GenerateDomKeyboardLayoutMap();
#else
  CR_NOTIMPLEMENTED();
  return {};
#endif
}

void WindowTreeHostPlatform::SetCursorNative(gfx::NativeCursor cursor) {
  if (cursor == current_cursor_)
    return;
  current_cursor_ = cursor;

#if defined(MINI_CHROMIUM_OS_WIN)
  crui::CursorLoaderWin cursor_loader;
  cursor_loader.SetPlatformCursor(&cursor);
#endif

  platform_window_->SetCursor(cursor.platform());
}

void WindowTreeHostPlatform::MoveCursorToScreenLocationInPixels(
    const gfx::Point& location_in_pixels) {
  platform_window_->MoveCursorTo(location_in_pixels);
}

void WindowTreeHostPlatform::OnCursorVisibilityChangedNative(bool show) {
  CR_NOTIMPLEMENTED();
}

void WindowTreeHostPlatform::OnBoundsChanged(const gfx::Rect& new_bounds) {
  // It's possible this function may be called recursively. Only notify
  // observers on initial entry. This way observers can safely assume that
  // OnHostDidProcessBoundsChange() is called when all bounds changes have
  // completed.
  if (++on_bounds_changed_recursion_depth_ == 1) {
    for (WindowTreeHostObserver& observer : observers())
      observer.OnHostWillProcessBoundsChange(this);
  }
  ///float current_scale = compositor()->device_scale_factor();
  float current_scale = 1.0f;
  float new_scale = crui::GetScaleFactorForNativeView(window());
  gfx::Rect old_bounds = bounds_in_pixels_;
  bounds_in_pixels_ = new_bounds;
  if (bounds_in_pixels_.origin() != old_bounds.origin())
    OnHostMovedInPixels(bounds_in_pixels_.origin());
  if (bounds_in_pixels_.size() != old_bounds.size() ||
      current_scale != new_scale) {
    pending_size_ = gfx::Size();
    OnHostResizedInPixels(bounds_in_pixels_.size());
  }
  CR_DCHECK(on_bounds_changed_recursion_depth_ > 0);
  if (--on_bounds_changed_recursion_depth_ == 0) {
    for (WindowTreeHostObserver& observer : observers())
      observer.OnHostDidProcessBoundsChange(this);
  }
}

void WindowTreeHostPlatform::OnDamageRect(const gfx::Rect& damage_rect) {
  ///compositor()->ScheduleRedrawRect(damage_rect);
}

void WindowTreeHostPlatform::DispatchEvent(crui::Event* event) {
  ////TRACE_EVENT0("input", "WindowTreeHostPlatform::DispatchEvent");
  crui::EventDispatchDetails details = SendEventToSink(event);
  if (details.dispatcher_destroyed)
    event->SetHandled();
}

void WindowTreeHostPlatform::OnCloseRequest() {
  OnHostCloseRequested();
}

void WindowTreeHostPlatform::OnClosed() {}

void WindowTreeHostPlatform::OnWindowStateChanged(
    crui::PlatformWindowState new_state) {}

void WindowTreeHostPlatform::OnLostCapture() {
  OnHostLostWindowCapture();
}

void WindowTreeHostPlatform::OnAcceleratedWidgetAvailable(
    gfx::AcceleratedWidget widget) {
  widget_ = widget;
  // This may be called before the Compositor has been created.
  ///if (compositor())
  ///  WindowTreeHost::OnAcceleratedWidgetAvailable();
}

void WindowTreeHostPlatform::OnAcceleratedWidgetDestroyed() {
  ///gfx::AcceleratedWidget widget = compositor()->ReleaseAcceleratedWidget();
  ///CR_DCHECK(widget == widget_);
  widget_ = gfx::kNullAcceleratedWidget;
}

void WindowTreeHostPlatform::OnActivationChanged(bool active) {}

void WindowTreeHostPlatform::OnMouseEnter() {
  client::CursorClient* cursor_client = client::GetCursorClient(window());
  if (cursor_client) {
    auto display =
        display::Screen::GetScreen()->GetDisplayNearestWindow(window());
    CR_DCHECK(display.is_valid());
    cursor_client->SetDisplay(display);
  }
}

}  // namespace aura
}  // namespace crui

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_window_tree_host_win.h"

#include "crbase/functional/bind.h"
#include "crbase/containers/flat_set.h"
#include "crbase/memory/ptr_util.h"
///#include "base/metrics/histogram_macros.h"
#include "crbase/win/win_util.h"
#include "crbase/win/windows_version.h"
//#include "third_party/skia/include/core/SkPath.h"
///#include "third_party/skia/include/core/SkRegion.h"
#include "crui/aura/client/aura_constants.h"
#include "crui/aura/client/cursor_client.h"
#include "crui/aura/client/focus_client.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/base/class_property.h"
#include "crui/base/cursor/cursor_loader_win.h"
///#include "crui/base/ime/input_method.h"
///#include "ui/base/ui_base_features.h"
#include "crui/base/win/shell.h"
///#include "ui/compositor/paint_context.h"
#include "crui/display/win/dpi.h"
#include "crui/display/win/screen_win.h"
#include "crui/events/keyboard_hook.h"
#include "crui/events/keycodes/dom/dom_code.h"
#include "crui/events/keycodes/dom/dom_keyboard_layout_map.h"
#include "crui/gfx/geometry/insets.h"
#include "crui/gfx/geometry/vector2d.h"
#include "crui/gfx/native_widget_types.h"
///#include "crui/gfx/path_win.h"
///#include "crui/views/corewm/tooltip_aura.h"
#include "crui/views/corewm/tooltip_win.h"
///#include "crui/views/views_features.h"
///#include "ui/views/views_switches.h"
#include "crui/views/widget/desktop_aura/desktop_drag_drop_client_win.h"
#include "crui/views/widget/desktop_aura/desktop_native_cursor_manager.h"
#include "crui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#include "crui/views/widget/root_view.h"
#include "crui/views/widget/widget_delegate.h"
#include "crui/views/widget/widget_hwnd_utils.h"
#include "crui/views/win/fullscreen_handler.h"
#include "crui/views/win/hwnd_message_handler.h"
#include "crui/views/win/hwnd_util.h"
#include "crui/views/window/native_frame_view.h"
#include "crui/aura/wm/core/compound_event_filter.h"
#include "crui/aura/wm/core/window_animations.h"
#include "crui/aura/wm/public/scoped_tooltip_disabler.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::views::DesktopWindowTreeHostWin*)

namespace crui {
namespace views {

namespace {

gfx::Size GetExpandedWindowSize(bool is_translucent, gfx::Size size) {
  if (!is_translucent || !crui::win::IsAeroGlassEnabled())
    return size;

  // Some AMD drivers can't display windows that are less than 64x64 pixels,
  // so expand them to be at least that size. http://crbug.com/286609
  gfx::Size expanded(std::max(size.width(), 64), std::max(size.height(), 64));
  return expanded;
}

void InsetBottomRight(gfx::Rect* rect, const gfx::Vector2d& vector) {
  rect->Inset(0, 0, vector.x(), vector.y());
}

}  // namespace

DEFINE_UI_CLASS_PROPERTY_KEY(aura::Window*, kContentWindowForRootWindow, NULL)

// Identifies the DesktopWindowTreeHostWin associated with the
// WindowEventDispatcher.
DEFINE_UI_CLASS_PROPERTY_KEY(DesktopWindowTreeHostWin*,
                             kDesktopWindowTreeHostKey,
                             NULL)

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWin, public:

bool DesktopWindowTreeHostWin::is_cursor_visible_ = true;

DesktopWindowTreeHostWin::DesktopWindowTreeHostWin(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura)
    : message_handler_(new HWNDMessageHandler(
          this,
          native_widget_delegate->AsWidget()->GetName())),
      native_widget_delegate_(native_widget_delegate),
      desktop_native_widget_aura_(desktop_native_widget_aura),
      drag_drop_client_(nullptr),
      should_animate_window_close_(false),
      pending_close_(false),
      has_non_client_view_(false),
      tooltip_(nullptr) {}

DesktopWindowTreeHostWin::~DesktopWindowTreeHostWin() {
  desktop_native_widget_aura_->OnDesktopWindowTreeHostDestroyed(this);
  DestroyDispatcher();
}

// static
aura::Window* DesktopWindowTreeHostWin::GetContentWindowForHWND(HWND hwnd) {
  // All HWND's we create should have WindowTreeHost instances associated with
  // them. There are exceptions like the content layer creating HWND's which
  // are not associated with WindowTreeHost instances.
  aura::WindowTreeHost* host =
      aura::WindowTreeHost::GetForAcceleratedWidget(hwnd);
  return host ? host->window()->GetProperty(kContentWindowForRootWindow) : NULL;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWin, DesktopWindowTreeHost implementation:

void DesktopWindowTreeHostWin::Init(const Widget::InitParams& params) {
  wants_mouse_events_when_inactive_ = params.wants_mouse_events_when_inactive;

  wm::SetAnimationHost(content_window(), this);
  if (params.type == Widget::InitParams::TYPE_WINDOW &&
      !params.remove_standard_frame)
    content_window()->SetProperty(aura::client::kAnimationsDisabledKey, true);

  ConfigureWindowStyles(message_handler_.get(), params,
                        GetWidget()->widget_delegate(),
                        native_widget_delegate_);

  HWND parent_hwnd = nullptr;
  if (params.parent && params.parent->GetHost())
    parent_hwnd = params.parent->GetHost()->GetAcceleratedWidget();

  remove_standard_frame_ = params.remove_standard_frame;
  has_non_client_view_ = Widget::RequiresNonClientView(params.type);
  z_order_ = params.EffectiveZOrderLevel();

  // We don't have an HWND yet, so scale relative to the nearest screen.
  gfx::Rect pixel_bounds =
      display::win::ScreenWin::DIPToScreenRect(nullptr, params.bounds);
  message_handler_->Init(parent_hwnd, pixel_bounds);
  CreateCompositor(/*viz::FrameSinkId(),*/ params.force_software_compositing);
  OnAcceleratedWidgetAvailable();
  InitHost();
  window()->Show();
}

void DesktopWindowTreeHostWin::OnNativeWidgetCreated(
    const Widget::InitParams& params) {
  // The cursor is not necessarily visible when the root window is created.
  aura::client::CursorClient* cursor_client =
      aura::client::GetCursorClient(window());
  if (cursor_client)
    is_cursor_visible_ = cursor_client->IsCursorVisible();

  window()->SetProperty(kContentWindowForRootWindow, content_window());
  window()->SetProperty(kDesktopWindowTreeHostKey, this);

  should_animate_window_close_ =
      content_window()->type() != aura::client::WINDOW_TYPE_NORMAL &&
      !wm::WindowAnimationsDisabled(content_window());
}

void DesktopWindowTreeHostWin::OnActiveWindowChanged(bool active) {}

void DesktopWindowTreeHostWin::OnWidgetInitDone() {}

std::unique_ptr<corewm::Tooltip> DesktopWindowTreeHostWin::CreateTooltip() {
  ///bool force_legacy_tooltips =
  ///    (cr::win::GetVersion() < cr::win::Version::WIN8);
  ///if (!force_legacy_tooltips)
  ///  return std::make_unique<corewm::TooltipAura>();

  CR_DCHECK(!tooltip_);
  tooltip_ = new corewm::TooltipWin(GetAcceleratedWidget());
  return cr::WrapUnique(tooltip_);
}

std::unique_ptr<aura::client::DragDropClient>
DesktopWindowTreeHostWin::CreateDragDropClient(
    DesktopNativeCursorManager* cursor_manager) {
  drag_drop_client_ = new DesktopDragDropClientWin(window(), GetHWND());
  return cr::WrapUnique(drag_drop_client_);
}

void DesktopWindowTreeHostWin::Close() {
  content_window()->Hide();
  // TODO(beng): Move this entire branch to DNWA so it can be shared with X11.
  if (should_animate_window_close_) {
    pending_close_ = true;
    const bool is_animating = false;
    ///    content_window()->layer()->GetAnimator()->IsAnimatingProperty(
    ///        ui::LayerAnimationElement::VISIBILITY);
    // Animation may not start for a number of reasons.
    if (!is_animating)
      message_handler_->Close();
    // else case, OnWindowHidingAnimationCompleted does the actual Close.
  } else {
    message_handler_->Close();
  }
}

void DesktopWindowTreeHostWin::CloseNow() {
  message_handler_->CloseNow();
}

aura::WindowTreeHost* DesktopWindowTreeHostWin::AsWindowTreeHost() {
  return this;
}

void DesktopWindowTreeHostWin::Show(crui::WindowShowState show_state,
                                    const gfx::Rect& restore_bounds) {
  ///if (compositor())
  ///  compositor()->SetVisible(true);

  gfx::Rect pixel_restore_bounds;
  if (show_state == crui::SHOW_STATE_MAXIMIZED) {
    pixel_restore_bounds =
        display::win::ScreenWin::DIPToScreenRect(GetHWND(), restore_bounds);
  }
  message_handler_->Show(show_state, pixel_restore_bounds);

  content_window()->Show();
}

bool DesktopWindowTreeHostWin::IsVisible() const {
  return message_handler_->IsVisible();
}

void DesktopWindowTreeHostWin::SetSize(const gfx::Size& size) {
  gfx::Size size_in_pixels = display::win::ScreenWin::DIPToScreenSize(GetHWND(),
                                                                      size);
  gfx::Size expanded =
      GetExpandedWindowSize(message_handler_->is_translucent(), size_in_pixels);
  window_enlargement_ =
      gfx::Vector2d(expanded.width() - size_in_pixels.width(),
                    expanded.height() - size_in_pixels.height());
  message_handler_->SetSize(expanded);
}

void DesktopWindowTreeHostWin::StackAbove(aura::Window* window) {
  HWND hwnd = HWNDForNativeView(window);
  if (hwnd)
    message_handler_->StackAbove(hwnd);
}

void DesktopWindowTreeHostWin::StackAtTop() {
  message_handler_->StackAtTop();
}

void DesktopWindowTreeHostWin::CenterWindow(const gfx::Size& size) {
  gfx::Size size_in_pixels = display::win::ScreenWin::DIPToScreenSize(GetHWND(),
                                                                      size);
  gfx::Size expanded_size;
  expanded_size =
      GetExpandedWindowSize(message_handler_->is_translucent(), size_in_pixels);
  window_enlargement_ =
      gfx::Vector2d(expanded_size.width() - size_in_pixels.width(),
                    expanded_size.height() - size_in_pixels.height());
  message_handler_->CenterWindow(expanded_size);
}

void DesktopWindowTreeHostWin::GetWindowPlacement(
    gfx::Rect* bounds,
    crui::WindowShowState* show_state) const {
  message_handler_->GetWindowPlacement(bounds, show_state);
  InsetBottomRight(bounds, window_enlargement_);
  *bounds = display::win::ScreenWin::ScreenToDIPRect(GetHWND(), *bounds);
}

gfx::Rect DesktopWindowTreeHostWin::GetWindowBoundsInScreen() const {
  gfx::Rect pixel_bounds = message_handler_->GetWindowBoundsInScreen();
  InsetBottomRight(&pixel_bounds, window_enlargement_);
  return display::win::ScreenWin::ScreenToDIPRect(GetHWND(), pixel_bounds);
}

gfx::Rect DesktopWindowTreeHostWin::GetClientAreaBoundsInScreen() const {
  gfx::Rect pixel_bounds = message_handler_->GetClientAreaBoundsInScreen();
  InsetBottomRight(&pixel_bounds, window_enlargement_);
  return display::win::ScreenWin::ScreenToDIPRect(GetHWND(), pixel_bounds);
}

gfx::Rect DesktopWindowTreeHostWin::GetRestoredBounds() const {
  gfx::Rect pixel_bounds = message_handler_->GetRestoredBounds();
  InsetBottomRight(&pixel_bounds, window_enlargement_);
  return display::win::ScreenWin::ScreenToDIPRect(GetHWND(), pixel_bounds);
}

std::string DesktopWindowTreeHostWin::GetWorkspace() const {
  return std::string();
}

gfx::Rect DesktopWindowTreeHostWin::GetWorkAreaBoundsInScreen() const {
  MONITORINFO monitor_info;
  monitor_info.cbSize = sizeof(monitor_info);
  GetMonitorInfo(MonitorFromWindow(message_handler_->hwnd(),
                                   MONITOR_DEFAULTTONEAREST),
                 &monitor_info);
  gfx::Rect pixel_bounds = gfx::Rect(monitor_info.rcWork);
  return display::win::ScreenWin::ScreenToDIPRect(GetHWND(), pixel_bounds);
}

void DesktopWindowTreeHostWin::SetShape(
    std::unique_ptr<Widget::ShapeRects> native_shape) {
  if (!native_shape || native_shape->empty()) {
    message_handler_->SetRegion(nullptr);
    return;
  }

  ///// TODO(wez): This would be a lot simpler if we were passed an SkPath.
  ///// See crbug.com/410593.
  ///SkRegion shape;
  ///const float scale = display::win::ScreenWin::GetScaleFactorForHWND(GetHWND());
  ///if (scale > 1.0) {
  ///  std::vector<SkIRect> sk_rects;
  ///  for (const gfx::Rect& rect : *native_shape) {
  ///    const SkIRect sk_rect = gfx::RectToSkIRect(rect);
  ///    SkRect scaled_rect =
  ///        SkRect::MakeLTRB(sk_rect.left() * scale, sk_rect.top() * scale,
  ///                         sk_rect.right() * scale, sk_rect.bottom() * scale);
  ///    SkIRect rounded_scaled_rect;
  ///    scaled_rect.roundOut(&rounded_scaled_rect);
  ///    sk_rects.push_back(rounded_scaled_rect);
  ///  }
  ///  shape.setRects(&sk_rects[0], sk_rects.size());
  ///} else {
  ///  for (const gfx::Rect& rect : *native_shape)
  ///    shape.op(gfx::RectToSkIRect(rect), SkRegion::kUnion_Op);
  ///}
  ///
  ///message_handler_->SetRegion(gfx::CreateHRGNFromSkRegion(shape));
}

void DesktopWindowTreeHostWin::Activate() {
  message_handler_->Activate();
}

void DesktopWindowTreeHostWin::Deactivate() {
  message_handler_->Deactivate();
}

bool DesktopWindowTreeHostWin::IsActive() const {
  return message_handler_->IsActive();
}

void DesktopWindowTreeHostWin::Maximize() {
  message_handler_->Maximize();
}

void DesktopWindowTreeHostWin::Minimize() {
  message_handler_->Minimize();
}

void DesktopWindowTreeHostWin::Restore() {
  message_handler_->Restore();
}

bool DesktopWindowTreeHostWin::IsMaximized() const {
  return message_handler_->IsMaximized();
}

bool DesktopWindowTreeHostWin::IsMinimized() const {
  return message_handler_->IsMinimized();
}

bool DesktopWindowTreeHostWin::HasCapture() const {
  return message_handler_->HasCapture();
}

void DesktopWindowTreeHostWin::SetZOrderLevel(crui::ZOrderLevel order) {
  z_order_ = order;
  // Emulate the multiple window levels provided by other platforms by
  // collapsing the z-order enum into kNormal = normal, everything else = always
  // on top.
  message_handler_->SetAlwaysOnTop(order != crui::ZOrderLevel::kNormal);
}

crui::ZOrderLevel DesktopWindowTreeHostWin::GetZOrderLevel() const {
  bool window_always_on_top = message_handler_->IsAlwaysOnTop();
  bool level_always_on_top = z_order_ != crui::ZOrderLevel::kNormal;

  if (window_always_on_top == level_always_on_top)
    return z_order_;

  // If something external has forced a window to be always-on-top, map it to
  // kFloatingWindow as a reasonable equivalent.
  return window_always_on_top ? crui::ZOrderLevel::kFloatingWindow
                              : crui::ZOrderLevel::kNormal;
}

void DesktopWindowTreeHostWin::SetVisibleOnAllWorkspaces(bool always_visible) {
  // Chrome does not yet support Windows 10 desktops.
}

bool DesktopWindowTreeHostWin::IsVisibleOnAllWorkspaces() const {
  return false;
}

bool DesktopWindowTreeHostWin::SetWindowTitle(const cr::string16& title) {
  return message_handler_->SetTitle(title);
}

void DesktopWindowTreeHostWin::ClearNativeFocus() {
  message_handler_->ClearNativeFocus();
}

Widget::MoveLoopResult DesktopWindowTreeHostWin::RunMoveLoop(
    const gfx::Vector2d& drag_offset,
    Widget::MoveLoopSource source,
    Widget::MoveLoopEscapeBehavior escape_behavior) {
  const bool hide_on_escape =
      escape_behavior == Widget::MOVE_LOOP_ESCAPE_BEHAVIOR_HIDE;
  return message_handler_->RunMoveLoop(drag_offset, hide_on_escape) ?
      Widget::MOVE_LOOP_SUCCESSFUL : Widget::MOVE_LOOP_CANCELED;
}

void DesktopWindowTreeHostWin::EndMoveLoop() {
  message_handler_->EndMoveLoop();
}

void DesktopWindowTreeHostWin::SetVisibilityChangedAnimationsEnabled(
    bool value) {
  message_handler_->SetVisibilityChangedAnimationsEnabled(value);
  content_window()->SetProperty(aura::client::kAnimationsDisabledKey, !value);
}

NonClientFrameView* DesktopWindowTreeHostWin::CreateNonClientFrameView() {
  return ShouldUseNativeFrame()
             ? new NativeFrameView(native_widget_delegate_->AsWidget())
             : nullptr;
}

bool DesktopWindowTreeHostWin::ShouldUseNativeFrame() const {
  return IsTranslucentWindowOpacitySupported();
}

bool DesktopWindowTreeHostWin::ShouldWindowContentsBeTransparent() const {
  // The window contents need to be transparent when the titlebar area is drawn
  // by the DWM rather than Chrome, so that area can show through.  This
  // function does not describe the transparency of the whole window appearance,
  // but merely of the content Chrome draws, so even when the system titlebars
  // appear opaque (Win 8+), the content above them needs to be transparent, or
  // they'll be covered by a black (undrawn) region.
  return ShouldUseNativeFrame() && !IsFullscreen();
}

void DesktopWindowTreeHostWin::FrameTypeChanged() {
  message_handler_->FrameTypeChanged();
}

void DesktopWindowTreeHostWin::SetFullscreen(bool fullscreen) {
  message_handler_->SetFullscreen(fullscreen);
  // TODO(sky): workaround for ScopedFullscreenVisibility showing window
  // directly. Instead of this should listen for visibility changes and then
  // update window.
  if (message_handler_->IsVisible() && !content_window()->TargetVisibility()) {
    ///if (compositor())
    ///  compositor()->SetVisible(true);
    content_window()->Show();
  }
  desktop_native_widget_aura_->UpdateWindowTransparency();
}

bool DesktopWindowTreeHostWin::IsFullscreen() const {
  return message_handler_->fullscreen_handler()->fullscreen();
}

void DesktopWindowTreeHostWin::SetOpacity(float opacity) {
  content_window()->layer()->SetOpacity(opacity);
}

void DesktopWindowTreeHostWin::SetAspectRatio(const gfx::SizeF& aspect_ratio) {
  CR_DCHECK(!aspect_ratio.IsEmpty());
  message_handler_->SetAspectRatio(aspect_ratio.width() /
                                   aspect_ratio.height());
}

///void DesktopWindowTreeHostWin::SetWindowIcons(
///    const gfx::ImageSkia& window_icon, const gfx::ImageSkia& app_icon) {
///  message_handler_->SetWindowIcons(window_icon, app_icon);
///}

void DesktopWindowTreeHostWin::InitModalType(crui::ModalType modal_type) {
  message_handler_->InitModalType(modal_type);
}

void DesktopWindowTreeHostWin::FlashFrame(bool flash_frame) {
  message_handler_->FlashFrame(flash_frame);
}

bool DesktopWindowTreeHostWin::IsAnimatingClosed() const {
  return pending_close_;
}

bool DesktopWindowTreeHostWin::IsTranslucentWindowOpacitySupported() const {
  return crui::win::IsAeroGlassEnabled();
}

void DesktopWindowTreeHostWin::SizeConstraintsChanged() {
  message_handler_->SizeConstraintsChanged();
}

bool DesktopWindowTreeHostWin::ShouldUpdateWindowTransparency() const {
  return true;
}

bool DesktopWindowTreeHostWin::ShouldUseDesktopNativeCursorManager() const {
  return true;
}

bool DesktopWindowTreeHostWin::ShouldCreateVisibilityController() const {
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWin, WindowTreeHost implementation:

crui::EventSource* DesktopWindowTreeHostWin::GetEventSource() {
  return this;
}

gfx::AcceleratedWidget DesktopWindowTreeHostWin::GetAcceleratedWidget() {
  return message_handler_->hwnd();
}

void DesktopWindowTreeHostWin::ShowImpl() {
  Show(crui::SHOW_STATE_NORMAL, gfx::Rect());
}

void DesktopWindowTreeHostWin::HideImpl() {
  if (!pending_close_)
    message_handler_->Hide();
}

// GetBoundsInPixels and SetBoundsInPixels work in pixel coordinates, whereas
// other get/set methods work in DIP.

gfx::Rect DesktopWindowTreeHostWin::GetBoundsInPixels() const {
  gfx::Rect bounds(message_handler_->GetClientAreaBounds());
  // If the window bounds were expanded we need to return the original bounds
  // To achieve this we do the reverse of the expansion, i.e. add the
  // window_expansion_top_left_delta_ to the origin and subtract the
  // window_expansion_bottom_right_delta_ from the width and height.
  gfx::Rect without_expansion(
      bounds.x() + window_expansion_top_left_delta_.x(),
      bounds.y() + window_expansion_top_left_delta_.y(),
      bounds.width() - window_expansion_bottom_right_delta_.x() -
          window_enlargement_.x(),
      bounds.height() - window_expansion_bottom_right_delta_.y() -
          window_enlargement_.y());
  return without_expansion;
}

void DesktopWindowTreeHostWin::SetBoundsInPixels(const gfx::Rect& bounds) {
  // If the window bounds have to be expanded we need to subtract the
  // window_expansion_top_left_delta_ from the origin and add the
  // window_expansion_bottom_right_delta_ to the width and height
  gfx::Size old_content_size = GetBoundsInPixels().size();

  gfx::Rect expanded(
      bounds.x() - window_expansion_top_left_delta_.x(),
      bounds.y() - window_expansion_top_left_delta_.y(),
      bounds.width() + window_expansion_bottom_right_delta_.x(),
      bounds.height() + window_expansion_bottom_right_delta_.y());

  gfx::Rect new_expanded(
      expanded.origin(),
      GetExpandedWindowSize(message_handler_->is_translucent(),
                            expanded.size()));
  window_enlargement_ =
      gfx::Vector2d(new_expanded.width() - expanded.width(),
                    new_expanded.height() - expanded.height());
  // When |new_expanded| causes the window to be moved to a display with a
  // different DSF, HWNDMessageHandler::OnDpiChanged() will be called and the
  // window size will be scaled automatically.
  message_handler_->SetBounds(new_expanded, old_content_size != bounds.size());
}

gfx::Point DesktopWindowTreeHostWin::GetLocationOnScreenInPixels() const {
  return GetBoundsInPixels().origin();
}

void DesktopWindowTreeHostWin::SetCapture() {
  message_handler_->SetCapture();
}

void DesktopWindowTreeHostWin::ReleaseCapture() {
  message_handler_->ReleaseCapture();
}

bool DesktopWindowTreeHostWin::CaptureSystemKeyEventsImpl(
    cr::Optional<cr::flat_set<crui::DomCode>> dom_codes) {
  // Only one KeyboardHook should be active at a time, otherwise there will be
  // problems with event routing (i.e. which Hook takes precedence) and
  // destruction ordering.
  CR_DCHECK(!keyboard_hook_);
  keyboard_hook_ = crui::KeyboardHook::CreateModifierKeyboardHook(
      std::move(dom_codes), GetAcceleratedWidget(),
      cr::BindRepeating(&DesktopWindowTreeHostWin::HandleKeyEvent,
                        cr::Unretained(this)));

  return keyboard_hook_ != nullptr;
}

void DesktopWindowTreeHostWin::ReleaseSystemKeyEventCapture() {
  keyboard_hook_.reset();
}

bool DesktopWindowTreeHostWin::IsKeyLocked(crui::DomCode dom_code) {
  return keyboard_hook_ && keyboard_hook_->IsKeyLocked(dom_code);
}

cr::flat_map<std::string, std::string>
DesktopWindowTreeHostWin::GetKeyboardLayoutMap() {
  return crui::GenerateDomKeyboardLayoutMap();
}

void DesktopWindowTreeHostWin::SetCursorNative(gfx::NativeCursor cursor) {
  crui::CursorLoaderWin cursor_loader;
  cursor_loader.SetPlatformCursor(&cursor);

  message_handler_->SetCursor(cursor.platform());
}

void DesktopWindowTreeHostWin::OnCursorVisibilityChangedNative(bool show) {
  if (is_cursor_visible_ == show)
    return;
  is_cursor_visible_ = show;
  ::ShowCursor(!!show);
}

void DesktopWindowTreeHostWin::MoveCursorToScreenLocationInPixels(
    const gfx::Point& location_in_pixels) {
  POINT cursor_location = location_in_pixels.ToPOINT();
  ::ClientToScreen(GetHWND(), &cursor_location);
  ::SetCursorPos(cursor_location.x, cursor_location.y);
}

std::unique_ptr<aura::ScopedEnableUnadjustedMouseEvents>
DesktopWindowTreeHostWin::RequestUnadjustedMovement() {
  return message_handler_->RegisterUnadjustedMouseEvent();
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWin, wm::AnimationHost implementation:

void DesktopWindowTreeHostWin::SetHostTransitionOffsets(
    const gfx::Vector2d& top_left_delta,
    const gfx::Vector2d& bottom_right_delta) {
  gfx::Rect bounds_without_expansion = GetBoundsInPixels();
  window_expansion_top_left_delta_ = top_left_delta;
  window_expansion_bottom_right_delta_ = bottom_right_delta;
  SetBoundsInPixels(bounds_without_expansion);
}

void DesktopWindowTreeHostWin::OnWindowHidingAnimationCompleted() {
  if (pending_close_)
    message_handler_->Close();
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWin, HWNDMessageHandlerDelegate implementation:

///ui::InputMethod* DesktopWindowTreeHostWin::GetHWNDMessageDelegateInputMethod() {
///  return GetInputMethod();
///}

bool DesktopWindowTreeHostWin::HasNonClientView() const {
  return has_non_client_view_;
}

FrameMode DesktopWindowTreeHostWin::GetFrameMode() const {
  return GetWidget()->ShouldUseNativeFrame() ? FrameMode::SYSTEM_DRAWN
                                             : FrameMode::CUSTOM_DRAWN;
}

bool DesktopWindowTreeHostWin::HasFrame() const {
  return !remove_standard_frame_;
}

void DesktopWindowTreeHostWin::SchedulePaint() {
  GetWidget()->GetRootView()->SchedulePaint();
}

bool DesktopWindowTreeHostWin::ShouldPaintAsActive() const {
  return GetWidget()->ShouldPaintAsActive();
}

bool DesktopWindowTreeHostWin::CanResize() const {
  return GetWidget()->widget_delegate()->CanResize();
}

bool DesktopWindowTreeHostWin::CanMaximize() const {
  return GetWidget()->widget_delegate()->CanMaximize();
}

bool DesktopWindowTreeHostWin::CanMinimize() const {
  return GetWidget()->widget_delegate()->CanMinimize();
}

bool DesktopWindowTreeHostWin::CanActivate() const {
  if (IsModalWindowActive())
    return true;
  return native_widget_delegate_->CanActivate();
}

bool DesktopWindowTreeHostWin::WantsMouseEventsWhenInactive() const {
  return wants_mouse_events_when_inactive_;
}

bool DesktopWindowTreeHostWin::WidgetSizeIsClientSize() const {
  const Widget* widget = GetWidget()->GetTopLevelWidget();
  return IsMaximized() || (widget && widget->ShouldUseNativeFrame());
}

bool DesktopWindowTreeHostWin::IsModal() const {
  return native_widget_delegate_->IsModal();
}

int DesktopWindowTreeHostWin::GetInitialShowState() const {
  return CanActivate() ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE;
}

bool DesktopWindowTreeHostWin::WillProcessWorkAreaChange() const {
  return GetWidget()->widget_delegate()->WillProcessWorkAreaChange();
}

int DesktopWindowTreeHostWin::GetNonClientComponent(
    const gfx::Point& point) const {
  gfx::Point dip_position = display::win::ScreenWin::ClientToDIPPoint(GetHWND(),
                                                                      point);
  return native_widget_delegate_->GetNonClientComponent(dip_position);
}

///void DesktopWindowTreeHostWin::GetWindowMask(const gfx::Size& size,
///                                             SkPath* path) {
///  if (GetWidget()->non_client_view()) {
///    GetWidget()->non_client_view()->GetWindowMask(size, path);
///  } else if (!window_enlargement_.IsZero()) {
///    gfx::Rect bounds(WidgetSizeIsClientSize()
///                         ? message_handler_->GetClientAreaBoundsInScreen()
///                         : message_handler_->GetWindowBoundsInScreen());
///    InsetBottomRight(&bounds, window_enlargement_);
///    path->addRect(SkRect::MakeXYWH(0, 0, bounds.width(), bounds.height()));
///  }
///}

bool DesktopWindowTreeHostWin::GetClientAreaInsets(gfx::Insets* insets,
                                                   HMONITOR monitor) const {
  return false;
}

bool DesktopWindowTreeHostWin::GetDwmFrameInsetsInPixels(
    gfx::Insets* insets) const {
  return false;
}

void DesktopWindowTreeHostWin::GetMinMaxSize(gfx::Size* min_size,
                                             gfx::Size* max_size) const {
  *min_size = native_widget_delegate_->GetMinimumSize();
  *max_size = native_widget_delegate_->GetMaximumSize();
}

gfx::Size DesktopWindowTreeHostWin::GetRootViewSize() const {
  return GetWidget()->GetRootView()->size();
}

gfx::Size DesktopWindowTreeHostWin::DIPToScreenSize(
    const gfx::Size& dip_size) const {
  return display::win::ScreenWin::DIPToScreenSize(GetHWND(), dip_size);
}

void DesktopWindowTreeHostWin::ResetWindowControls() {
  if (GetWidget()->non_client_view())
    GetWidget()->non_client_view()->ResetWindowControls();
}

void DesktopWindowTreeHostWin::PaintLayeredWindow(gfx::Canvas* canvas) {
  GetWidget()->GetRootView()->Paint(canvas, CullSet());
}

gfx::NativeViewAccessible DesktopWindowTreeHostWin::GetNativeViewAccessible() {
  // This function may be called during shutdown when the |RootView| is nullptr.
  return GetWidget()->GetRootView()
             ? GetWidget()->GetRootView()->GetNativeViewAccessible()
             : nullptr;
}

void DesktopWindowTreeHostWin::HandleActivationChanged(bool active) {
  // This can be invoked from HWNDMessageHandler::Init(), at which point we're
  // not in a good state and need to ignore it.
  // TODO(beng): Do we need this still now the host owns the dispatcher?
  if (!dispatcher())
    return;

  desktop_native_widget_aura_->HandleActivationChanged(active);
}

bool DesktopWindowTreeHostWin::HandleAppCommand(short command) {
  // We treat APPCOMMAND ids as an extension of our command namespace, and just
  // let the delegate figure out what to do...
  return GetWidget()->widget_delegate() &&
      GetWidget()->widget_delegate()->ExecuteWindowsCommand(command);
}

void DesktopWindowTreeHostWin::HandleCancelMode() {
  dispatcher()->DispatchCancelModeEvent();
}

void DesktopWindowTreeHostWin::HandleCaptureLost() {
  OnHostLostWindowCapture();
}

void DesktopWindowTreeHostWin::HandleClose() {
  GetWidget()->Close();
}

bool DesktopWindowTreeHostWin::HandleCommand(int command) {
  return GetWidget()->widget_delegate()->ExecuteWindowsCommand(command);
}

void DesktopWindowTreeHostWin::HandleAccelerator(
    const crui::Accelerator& accelerator) {
  GetWidget()->GetFocusManager()->ProcessAccelerator(accelerator);
}

void DesktopWindowTreeHostWin::HandleCreate() {
  native_widget_delegate_->OnNativeWidgetCreated();
}

void DesktopWindowTreeHostWin::HandleDestroying() {
  drag_drop_client_->OnNativeWidgetDestroying(GetHWND());
  native_widget_delegate_->OnNativeWidgetDestroying();

  // Destroy the compositor before destroying the HWND since shutdown
  // may try to swap to the window.
  DestroyCompositor();
}

void DesktopWindowTreeHostWin::HandleDestroyed() {
  desktop_native_widget_aura_->OnHostClosed();
}

bool DesktopWindowTreeHostWin::HandleInitialFocus(
    crui::WindowShowState show_state) {
  return GetWidget()->SetInitialFocus(show_state);
}

void DesktopWindowTreeHostWin::HandleDisplayChange() {
  GetWidget()->widget_delegate()->OnDisplayChanged();
}

void DesktopWindowTreeHostWin::HandleBeginWMSizeMove() {
  native_widget_delegate_->OnNativeWidgetBeginUserBoundsChange();
}

void DesktopWindowTreeHostWin::HandleEndWMSizeMove() {
  native_widget_delegate_->OnNativeWidgetEndUserBoundsChange();
}

void DesktopWindowTreeHostWin::HandleMove() {
  CheckForMonitorChange();
  native_widget_delegate_->OnNativeWidgetMove();
  OnHostMovedInPixels(GetBoundsInPixels().origin());
}

void DesktopWindowTreeHostWin::HandleWorkAreaChanged() {
  CheckForMonitorChange();
  GetWidget()->widget_delegate()->OnWorkAreaChanged();
}

void DesktopWindowTreeHostWin::HandleVisibilityChanging(bool visible) {
  native_widget_delegate_->OnNativeWidgetVisibilityChanging(visible);
}

void DesktopWindowTreeHostWin::HandleVisibilityChanged(bool visible) {
  native_widget_delegate_->OnNativeWidgetVisibilityChanged(visible);
}

void DesktopWindowTreeHostWin::HandleWindowMinimizedOrRestored(bool restored) {
  // Ignore minimize/restore events that happen before widget initialization is
  // done. If a window is created minimized, and then activated, restoring
  // focus will fail because the root window is not visible, which is exposed by
  // ExtensionWindowCreateTest.AcceptState.
  if (!native_widget_delegate_->IsNativeWidgetInitialized())
    return;

  if (restored)
    window()->Show();
  else
    window()->Hide();
}

void DesktopWindowTreeHostWin::HandleClientSizeChanged(
    const gfx::Size& new_size) {
  CheckForMonitorChange();
  if (dispatcher())
    OnHostResizedInPixels(new_size);
}

void DesktopWindowTreeHostWin::HandleFrameChanged() {
  CheckForMonitorChange();
  desktop_native_widget_aura_->UpdateWindowTransparency();
  // Replace the frame and layout the contents.
  if (GetWidget()->non_client_view())
    GetWidget()->non_client_view()->UpdateFrame();
}

void DesktopWindowTreeHostWin::HandleNativeFocus(HWND last_focused_window) {
  // TODO(beng): inform the native_widget_delegate_.
}

void DesktopWindowTreeHostWin::HandleNativeBlur(HWND focused_window) {
  // TODO(beng): inform the native_widget_delegate_.
}

bool DesktopWindowTreeHostWin::HandleMouseEvent(crui::MouseEvent* event) {
  // Mouse events in occluded windows should be very rare. If this stat isn't
  // very close to 0, that would indicate that windows are incorrectly getting
  // marked occluded, or getting stuck in the occluded state. Event can cause
  // this object to be deleted so check occlusion state before we do anything
  // with the event.
  ///if (GetNativeWindowOcclusionState() == aura::Window::OcclusionState::OCCLUDED)
  ///  UMA_HISTOGRAM_BOOLEAN("OccludedWindowMouseEvents", true);

  SendEventToSink(event);
  return event->handled();
}

void DesktopWindowTreeHostWin::HandleKeyEvent(crui::KeyEvent* event) {
  // Bypass normal handling of alt-space, which would otherwise consume the
  // corresponding WM_SYSCHAR.  This allows HandleIMEMessage() to show the
  // system menu in this case.  If we instead showed the system menu here, the
  // WM_SYSCHAR would trigger a beep when processed by the native event handler.
  if ((event->type() == crui::ET_KEY_PRESSED) &&
      (event->key_code() == crui::VKEY_SPACE) &&
      (event->flags() & crui::EF_ALT_DOWN) &&
      !(event->flags() & crui::EF_CONTROL_DOWN) &&
      GetWidget()->non_client_view()) {
    return;
  }

  SendEventToSink(event);
}

void DesktopWindowTreeHostWin::HandleTouchEvent(crui::TouchEvent* event) {
  // HWNDMessageHandler asynchronously processes touch events. Because of this
  // it's possible for the aura::WindowEventDispatcher to have been destroyed
  // by the time we attempt to process them.
  if (!GetWidget()->GetNativeView())
    return;

  // Currently we assume the window that has capture gets touch events too.
  aura::WindowTreeHost* host =
      aura::WindowTreeHost::GetForAcceleratedWidget(GetCapture());
  if (host) {
    DesktopWindowTreeHostWin* target =
        host->window()->GetProperty(kDesktopWindowTreeHostKey);
    if (target && target->HasCapture() && target != this) {
      POINT target_location(event->location().ToPOINT());
      ClientToScreen(GetHWND(), &target_location);
      ScreenToClient(target->GetHWND(), &target_location);
      crui::TouchEvent target_event(*event, static_cast<View*>(nullptr),
                                    static_cast<View*>(nullptr));
      target_event.set_location(gfx::Point(target_location));
      target_event.set_root_location(target_event.location());
      target->SendEventToSink(&target_event);
      return;
    }
  }
  SendEventToSink(event);
}

bool DesktopWindowTreeHostWin::HandleIMEMessage(UINT message,
                                                WPARAM w_param,
                                                LPARAM l_param,
                                                LRESULT* result) {
  // Show the system menu at an appropriate location on alt-space.
  if ((message == WM_SYSCHAR) && (w_param == VK_SPACE) &&
      GetWidget()->non_client_view()) {
    const auto* frame = GetWidget()->non_client_view()->frame_view();
    ShowSystemMenuAtScreenPixelLocation(
        GetHWND(), frame->GetSystemMenuScreenPixelLocation());
    return true;
  }

  ///MSG msg = {};
  ///msg.hwnd = GetHWND();
  ///msg.message = message;
  ///msg.wParam = w_param;
  ///msg.lParam = l_param;
  ///return GetInputMethod()->OnUntranslatedIMEMessage(msg, result);
  CR_NOTIMPLEMENTED() << "DesktopWindowTreeHostWin::HandleIMEMessage.";
  ///CR_DCHECK(false);
  return false;
}

void DesktopWindowTreeHostWin::HandleInputLanguageChange(
    DWORD character_set,
    HKL input_language_id) {
  ///GetInputMethod()->OnInputLocaleChanged();
}

bool DesktopWindowTreeHostWin::HandlePaintAccelerated(
    const gfx::Rect& invalid_rect) {
  ///if (compositor())
  ///  compositor()->ScheduleRedrawRect(invalid_rect);
  return false;
}

void DesktopWindowTreeHostWin::HandlePaint(gfx::Canvas* canvas) {
  native_widget_delegate_->OnNativeWidgetPaint(canvas);
}

bool DesktopWindowTreeHostWin::HandleTooltipNotify(int w_param,
                                                   NMHDR* l_param,
                                                   LRESULT* l_result) {
  return tooltip_ && tooltip_->HandleNotify(w_param, l_param, l_result);
}

void DesktopWindowTreeHostWin::HandleMenuLoop(bool in_menu_loop) {
  if (in_menu_loop) {
    tooltip_disabler_ = std::make_unique<wm::ScopedTooltipDisabler>(window());
  } else {
    tooltip_disabler_.reset();
  }
}

bool DesktopWindowTreeHostWin::PreHandleMSG(UINT message,
                                            WPARAM w_param,
                                            LPARAM l_param,
                                            LRESULT* result) {
  return false;
}

void DesktopWindowTreeHostWin::PostHandleMSG(UINT message,
                                             WPARAM w_param,
                                             LPARAM l_param) {
}

bool DesktopWindowTreeHostWin::HandleScrollEvent(crui::ScrollEvent* event) {
  SendEventToSink(event);
  return event->handled();
}

bool DesktopWindowTreeHostWin::HandleGestureEvent(crui::GestureEvent* event) {
  SendEventToSink(event);
  return event->handled();
}

void DesktopWindowTreeHostWin::HandleWindowSizeChanging() {
  ///if (compositor())
  ///  compositor()->DisableSwapUntilResize();
}

void DesktopWindowTreeHostWin::HandleWindowSizeUnchanged() {
  // A resize may not have occurred if the window size happened not to have
  // changed (can occur on Windows 10 when snapping a window to the side of
  // the screen). In that case do a resize to the current size to reenable
  // swaps.
  ///if (compositor())
  ///  compositor()->ReenableSwap();
}

void DesktopWindowTreeHostWin::HandleWindowScaleFactorChanged(
    float window_scale_factor) {
  // TODO(ccameron): This will violate surface invariants, and is insane.
  // Shouldn't the scale factor and window pixel size changes be sent
  // atomically? And how does this interact with updates to display::Display?
  // Should we expect the display::Display to be updated before this? If so,
  // why can't we use the DisplayObserver that the base WindowTreeHost is
  // using?
  ///if (compositor()) {
  ///  compositor()->SetScaleAndSize(
  ///      window_scale_factor, message_handler_->GetClientAreaBounds().size(),
  ///      window()->GetLocalSurfaceIdAllocation());
  ///}
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHostWin, private:

Widget* DesktopWindowTreeHostWin::GetWidget() {
  return native_widget_delegate_->AsWidget();
}

const Widget* DesktopWindowTreeHostWin::GetWidget() const {
  return native_widget_delegate_->AsWidget();
}

HWND DesktopWindowTreeHostWin::GetHWND() const {
  return message_handler_->hwnd();
}

bool DesktopWindowTreeHostWin::IsModalWindowActive() const {
  // This function can get called during window creation which occurs before
  // dispatcher() has been created.
  if (!dispatcher())
    return false;

  const auto is_active = [](const auto* child) {
    return child->GetProperty(aura::client::kModalKey) != crui::MODAL_TYPE_NONE 
        && child->TargetVisibility();
  };
  return std::any_of(window()->children().cbegin(), window()->children().cend(),
                     is_active);
}

void DesktopWindowTreeHostWin::CheckForMonitorChange() {
  HMONITOR monitor_from_window =
      ::MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTOPRIMARY);
  if (monitor_from_window == last_monitor_from_window_)
    return;
  last_monitor_from_window_ = monitor_from_window;
  OnHostDisplayChanged();
}

aura::Window* DesktopWindowTreeHostWin::content_window() {
  return desktop_native_widget_aura_->content_window();
}

////////////////////////////////////////////////////////////////////////////////
// DesktopWindowTreeHost, public:

// static
DesktopWindowTreeHost* DesktopWindowTreeHost::Create(
    internal::NativeWidgetDelegate* native_widget_delegate,
    DesktopNativeWidgetAura* desktop_native_widget_aura) {
  return new DesktopWindowTreeHostWin(native_widget_delegate,
                                      desktop_native_widget_aura);
}

}  // namespace views
}  // namespace crui

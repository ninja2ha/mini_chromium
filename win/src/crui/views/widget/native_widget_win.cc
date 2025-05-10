// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/native_widget_win.h"

#include "crbase/logging.h"
#include "crui/base/view_prop.h"
#include "crui/base/win/shell.h"
#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/geometry/size.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/point_f.h"
#include "crui/display/display.h"
#include "crui/display/win/screen_win.h"
#include "crui/views/widget/widget_hwnd_utils.h"
#include "crui/views/widget/widget_delegate.h"
#include "crui/views/win/hwnd_message_handler.h"
#include "crui/views/win/fullscreen_handler.h"

namespace crui {
namespace views {

namespace {

// Links the HWND to its NativeWidget.
const char* const kNativeWidgetKey = "__VIEWS_NATIVE_WIDGET__";

// Enumeration callback for NativeWidget::GetAllChildWidgets(). Called for each
// child HWND beneath the original HWND.
BOOL CALLBACK EnumerateChildWindowsForNativeWidgets(HWND hwnd, LPARAM l_param) {
  Widget* widget = Widget::GetWidgetForNativeView(hwnd);
  if (widget) {
    Widget::Widgets* widgets = reinterpret_cast<Widget::Widgets*>(l_param);
    widgets->insert(widget);
  }
  return TRUE;
}

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

namespace internal {

// static
NativeWidgetPrivate* NativeWidgetPrivate::CreateNativeWidget(
    internal::NativeWidgetDelegate* delegate) {
  return new NativeWidgetWin(delegate);
}

// static 
NativeWidgetPrivate* NativeWidgetPrivate::GetNativeWidgetForNativeView(
    gfx::NativeView native_view) {
  return reinterpret_cast<NativeWidgetWin*>(
      ViewProp::GetValue(native_view, kNativeWidgetKey));
}

// static 
NativeWidgetPrivate* NativeWidgetPrivate::GetNativeWidgetForNativeWindow(
    gfx::NativeWindow native_window) {
  return GetNativeWidgetForNativeView(native_window);
}

// static
NativeWidgetPrivate* NativeWidgetPrivate::GetTopLevelNativeWidget(
    gfx::NativeView native_view) {
  if (!native_view)
    return NULL;

  // First, check if the top-level window is a Widget.
  HWND root = ::GetAncestor(native_view, GA_ROOT);
  if (!root)
    return NULL;

  NativeWidgetPrivate* widget = GetNativeWidgetForNativeView(root);
  if (widget)
    return widget;

  // Second, try to locate the last Widget window in the parent hierarchy.
  HWND parent_hwnd = native_view;
  // If we fail to find the native widget pointer for the root then it probably
  // means that the root belongs to a different process in which case we walk up
  // the native view chain looking for a parent window which corresponds to a
  // valid native widget. We only do this if we fail to find the native widget
  // for the current native view which means it is being destroyed.
  if (!widget && !GetNativeWidgetForNativeView(native_view)) {
    parent_hwnd = ::GetAncestor(parent_hwnd, GA_PARENT);
    if (!parent_hwnd)
      return NULL;
  }
  NativeWidgetPrivate* parent_widget;
  do {
    parent_widget = GetNativeWidgetForNativeView(parent_hwnd);
    if (parent_widget) {
      widget = parent_widget;
      parent_hwnd = ::GetAncestor(parent_hwnd, GA_PARENT);
    }
  } while (parent_hwnd != NULL && parent_widget != NULL);

  return widget;
}

// static
void NativeWidgetPrivate::GetAllChildWidgets(gfx::NativeView native_view,
                                             Widget::Widgets* children) {
  if (!native_view)
    return;

  Widget* widget = Widget::GetWidgetForNativeView(native_view);
  if (widget)
    children->insert(widget);
  ::EnumChildWindows(native_view, EnumerateChildWindowsForNativeWidgets,
                     reinterpret_cast<LPARAM>(children));
}

// static 
void NativeWidgetPrivate::GetAllOwnedWidgets(gfx::NativeView native_view,
                                             Widget::Widgets* owned) {
  CR_NOTIMPLEMENTED();
}

// static
void NativeWidgetPrivate::ReparentNativeView(gfx::NativeView native_view,
                                             gfx::NativeView new_parent) {
  if (!native_view)
    return;

  HWND previous_parent = ::GetParent(native_view);
  if (previous_parent == new_parent)
    return;

  Widget::Widgets widgets;
  GetAllChildWidgets(native_view, &widgets);

  // First notify all the widgets that they are being disassociated
  // from their previous parent.
  for (Widget::Widgets::iterator it = widgets.begin();
       it != widgets.end(); ++it) {
    // TODO(beng): Rename this notification to NotifyNativeViewChanging()
    // and eliminate the bool parameter.
    (*it)->NotifyNativeViewHierarchyWillChange();
  }

  ::SetParent(native_view, new_parent);

  // And now, notify them that they have a brand new parent.
  for (Widget::Widgets::iterator it = widgets.begin();
       it != widgets.end(); ++it) {
    (*it)->NotifyNativeViewHierarchyChanged();
  }
}

}  // namespace internal

NativeWidgetWin::NativeWidgetWin(internal::NativeWidgetDelegate* delegate) 
    : native_widget_delegate_(delegate),
      message_handler_(new HWNDMessageHandler(this, std::string())) {
}

NativeWidgetWin::~NativeWidgetWin() {
  if (ownership_ == Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET)
    delete native_widget_delegate_;
  else
    CloseNow();
  message_handler_.reset();
}

void NativeWidgetWin::InitNativeWidget(Widget::InitParams params) {
  ownership_ = params.ownership;
  wants_mouse_events_when_inactive_ = params.wants_mouse_events_when_inactive;

  ConfigureWindowStyles(message_handler_.get(), params,
                        GetWidget()->widget_delegate(),
                        native_widget_delegate_);

  remove_standard_frame_ = params.remove_standard_frame;
  has_non_client_view_ = Widget::RequiresNonClientView(params.type);
  z_order_ = params.EffectiveZOrderLevel();

  // We don't have an HWND yet, so scale relative to the nearest screen.
  gfx::Rect pixel_bounds =
      display::win::ScreenWin::DIPToScreenRect(nullptr, params.bounds);
  message_handler_->Init(params.parent, pixel_bounds);
}

void NativeWidgetWin::OnWidgetInitDone() {
}

bool NativeWidgetWin::ShouldUseNativeFrame() const {
  return true;
}

bool NativeWidgetWin::ShouldWindowContentsBeTransparent() const {
  // The window contents need to be transparent when the titlebar area is drawn
  // by the DWM rather than Chrome, so that area can show through.  This
  // function does not describe the transparency of the whole window appearance,
  // but merely of the content Chrome draws, so even when the system titlebars
  // appear opaque (Win 8+), the content above them needs to be transparent, or
  // they'll be covered by a black (undrawn) region.
  return crui::win::IsAeroGlassEnabled() && !IsFullscreen();
}

void NativeWidgetWin::FrameTypeChanged() {
  // This is called when the Theme has changed; forward the event to the root
  // widget.
  message_handler_->FrameTypeChanged();
}

gfx::NativeView NativeWidgetWin::GetNativeView() const {
  return message_handler_->hwnd();
}

gfx::NativeWindow NativeWidgetWin::GetNativeWindow() const {
  return message_handler_->hwnd();
}

Widget* NativeWidgetWin::GetTopLevelWidget() {
  NativeWidgetPrivate* native_widget = GetTopLevelNativeWidget(GetNativeView());
  return native_widget ? native_widget->GetWidget() : NULL;
}

void NativeWidgetWin::ReorderNativeViews() {
  
}

void NativeWidgetWin::SetNativeWindowProperty(const char* key, void* value) {
  // Remove the existing property (if any).
  for (ViewProps::iterator i = props_.begin(); i != props_.end(); ++i) {
    if ((*i)->Key() == key) {
      props_.erase(i);
      break;
    }
  }

  if (value)
    props_.push_back(std::make_unique<ViewProp>(GetNativeView(), key, value));
}

void* NativeWidgetWin::GetNativeWindowProperty(const char* key) const {
  return ViewProp::GetValue(GetNativeView(), key);
}

void NativeWidgetWin::SetCapture() {
  message_handler_->SetCapture();
}

void NativeWidgetWin::ReleaseCapture() {
  message_handler_->ReleaseCapture();
}

bool NativeWidgetWin::HasCapture() const {
  return message_handler_->HasCapture();
}

void NativeWidgetWin::CenterWindow(const gfx::Size& size) {
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

void NativeWidgetWin::GetWindowPlacement(
    gfx::Rect* bounds,
    crui::WindowShowState* show_state) const {
  message_handler_->GetWindowPlacement(bounds, show_state);
  InsetBottomRight(bounds, window_enlargement_);
  *bounds = display::win::ScreenWin::ScreenToDIPRect(GetHWND(), *bounds);
}

bool NativeWidgetWin::SetWindowTitle(const cr::string16& title) {
  return message_handler_->SetTitle(title);
}

void NativeWidgetWin::InitModalType(crui::ModalType modal_type) {
  message_handler_->InitModalType(modal_type);
}

gfx::Rect NativeWidgetWin::GetWindowBoundsInScreen() const {
  gfx::Rect pixel_bounds = message_handler_->GetWindowBoundsInScreen();
  InsetBottomRight(&pixel_bounds, window_enlargement_);
  return display::win::ScreenWin::ScreenToDIPRect(GetHWND(), pixel_bounds);
}

gfx::Rect NativeWidgetWin::GetClientAreaBoundsInScreen() const {
  gfx::Rect pixel_bounds = message_handler_->GetClientAreaBoundsInScreen();
  InsetBottomRight(&pixel_bounds, window_enlargement_);
  return display::win::ScreenWin::ScreenToDIPRect(GetHWND(), pixel_bounds);
}

gfx::Rect NativeWidgetWin::GetRestoredBounds() const {
  gfx::Rect pixel_bounds = message_handler_->GetRestoredBounds();
  InsetBottomRight(&pixel_bounds, window_enlargement_);
  return display::win::ScreenWin::ScreenToDIPRect(GetHWND(), pixel_bounds);
}

std::string NativeWidgetWin::GetWorkspace() const {
  return std::string();
}

void NativeWidgetWin::SetBounds(const gfx::Rect& bounds) {
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

void NativeWidgetWin::SetBoundsConstrained(const gfx::Rect& bounds) {

}

void NativeWidgetWin::SetSize(const gfx::Size& size) {
  gfx::Size size_in_pixels = display::win::ScreenWin::DIPToScreenSize(
      GetHWND(), size);
  gfx::Size expanded =
      GetExpandedWindowSize(message_handler_->is_translucent(), size_in_pixels);
  window_enlargement_ =
      gfx::Vector2d(expanded.width() - size_in_pixels.width(),
                    expanded.height() - size_in_pixels.height());
  message_handler_->SetSize(expanded);
}

void NativeWidgetWin::StackAbove(gfx::NativeView native_view) {
  if (native_view)
    message_handler_->StackAbove(native_view);
}

void NativeWidgetWin::StackAtTop() {
  message_handler_->StackAtTop();
}

void NativeWidgetWin::SetShape(std::unique_ptr<Widget::ShapeRects> shape) {
  if (!shape || shape->empty()) {
    message_handler_->SetRegion(nullptr);
    return;
  }
}

void NativeWidgetWin::Close() {
  message_handler_->Close();
}

void NativeWidgetWin::CloseNow() {
  message_handler_->CloseNow();
}

void NativeWidgetWin::Show(crui::WindowShowState show_state,
                            const gfx::Rect& restore_bounds) {
  ///if (compositor())
  ///  compositor()->SetVisible(true);

  gfx::Rect pixel_restore_bounds;
  if (show_state == crui::SHOW_STATE_MAXIMIZED) {
    pixel_restore_bounds =
        display::win::ScreenWin::DIPToScreenRect(GetHWND(), restore_bounds);
  }
  message_handler_->Show(show_state, pixel_restore_bounds);

  ///content_window()->Show();
}

void NativeWidgetWin::Hide() {
  return message_handler_->Hide();
}

bool NativeWidgetWin::IsVisible() const {
  return message_handler_->IsVisible();
}

void NativeWidgetWin::Activate() {
  message_handler_->Activate();
}

void NativeWidgetWin::Deactivate() {
  message_handler_->Deactivate();
}

bool NativeWidgetWin::IsActive() const {
  return message_handler_->IsActive();
}

void NativeWidgetWin::SetZOrderLevel(crui::ZOrderLevel order) {
  z_order_ = order;
  // Emulate the multiple window levels provided by other platforms by
  // collapsing the z-order enum into kNormal = normal, everything else = always
  // on top.
  message_handler_->SetAlwaysOnTop(order != crui::ZOrderLevel::kNormal);
}

crui::ZOrderLevel NativeWidgetWin::GetZOrderLevel() const {
  bool window_always_on_top = message_handler_->IsAlwaysOnTop();
  bool level_always_on_top = z_order_ != crui::ZOrderLevel::kNormal;

  if (window_always_on_top == level_always_on_top)
    return z_order_;

  // If something external has forced a window to be always-on-top, map it to
  // kFloatingWindow as a reasonable equivalent.
  return window_always_on_top ? crui::ZOrderLevel::kFloatingWindow
                              : crui::ZOrderLevel::kNormal;
}

void NativeWidgetWin::SetVisibleOnAllWorkspaces(bool always_visible) {
  // Chrome does not yet support Windows 10 desktops.
}

bool NativeWidgetWin::IsVisibleOnAllWorkspaces() const {
  return false;
}

void NativeWidgetWin::Maximize() {
  message_handler_->Maximize();
}

void NativeWidgetWin::Minimize() {
  message_handler_->Minimize();
}

bool NativeWidgetWin::IsMaximized() const {
  return message_handler_->IsMaximized();
}

bool NativeWidgetWin::IsMinimized() const {
  return message_handler_->IsMinimized();
}

void NativeWidgetWin::Restore() {
  message_handler_->Restore();
}

void NativeWidgetWin::SetFullscreen(bool fullscreen) {
  message_handler_->SetFullscreen(fullscreen);
  // TODO(sky): workaround for ScopedFullscreenVisibility showing window
  // directly. Instead of this should listen for visibility changes and then
  // update window.
  ///if (message_handler_->IsVisible() && !content_window()->TargetVisibility()) {
  ///  if (compositor())
  ///    compositor()->SetVisible(true);
  ///  content_window()->Show();
  ///}
  ///desktop_native_widget_aura_->UpdateWindowTransparency();
}

bool NativeWidgetWin::IsFullscreen() const {
  return message_handler_->IsFullscreen();
}

void NativeWidgetWin::SetCanAppearInExistingFullscreenSpaces(
    bool can_appear_in_existing_fullscreen_spaces) {

}

void NativeWidgetWin::SetOpacity(float opacity) {
  ///message_handler_->SetOpacity(static_cast<BYTE>(opacity));
  ///GetWidget()->GetRootView()->SchedulePaint();
}

void NativeWidgetWin::SetAspectRatio(const gfx::SizeF& aspect_ratio) {
  CR_DCHECK(!aspect_ratio.IsEmpty());
  message_handler_->SetAspectRatio(aspect_ratio.width() /
                                   aspect_ratio.height());
}

void NativeWidgetWin::FlashFrame(bool flash_frame) {
  message_handler_->FlashFrame(flash_frame);
}

void NativeWidgetWin::SchedulePaintInRect(const gfx::Rect& rect) {
  ///message_handler_->SchedulePaintInRect(rect);
}

void NativeWidgetWin::ScheduleLayout() {

}

void NativeWidgetWin::SetCursor(gfx::NativeCursor cursor) {
  message_handler_->SetCursor(cursor);
}

bool NativeWidgetWin::IsMouseEventsEnabled() const { 
  return true;
}

bool NativeWidgetWin::IsMouseButtonDown() const { 
  return (::GetKeyState(VK_LBUTTON) & 0x80) ||
    (::GetKeyState(VK_RBUTTON) & 0x80) ||
    (::GetKeyState(VK_MBUTTON) & 0x80) ||
    (::GetKeyState(VK_XBUTTON1) & 0x80) ||
    (::GetKeyState(VK_XBUTTON2) & 0x80);
}

void NativeWidgetWin::ClearNativeFocus() {
  message_handler_->ClearNativeFocus();
}

gfx::Rect NativeWidgetWin::GetWorkAreaBoundsInScreen() const { 
  return display::Screen::GetScreen()
      ->GetDisplayNearestWindow(GetNativeView())
      .work_area();
}

Widget::MoveLoopResult NativeWidgetWin::RunMoveLoop(
    const gfx::Vector2d& drag_offset,
    Widget::MoveLoopSource source,
    Widget::MoveLoopEscapeBehavior escape_behavior) { 
  return message_handler_->RunMoveLoop(
      drag_offset, 
      escape_behavior == Widget::MOVE_LOOP_ESCAPE_BEHAVIOR_HIDE) ?
          Widget::MOVE_LOOP_SUCCESSFUL : Widget::MOVE_LOOP_CANCELED;
}

void NativeWidgetWin::EndMoveLoop()  {
  message_handler_->EndMoveLoop();
}

void NativeWidgetWin::SetVisibilityChangedAnimationsEnabled(bool value) {
  message_handler_->SetVisibilityChangedAnimationsEnabled(value);
}

void NativeWidgetWin::SetVisibilityAnimationDuration(
    const cr::TimeDelta& duration) { 
}

void NativeWidgetWin::SetVisibilityAnimationTransition(
    Widget::VisibilityTransition transition) { 
}

bool NativeWidgetWin::IsTranslucentWindowOpacitySupported() const { 
  return true;
}

///crui::GestureRecognizer* GetGestureRecognizer() { }
void NativeWidgetWin::OnSizeConstraintsChanged() { 
}

void NativeWidgetWin::OnNativeViewHierarchyWillChange() { 
}

void NativeWidgetWin::OnNativeViewHierarchyChanged() { 
}

// Returns an internal name that matches the name of the associated Widget.
std::string NativeWidgetWin::GetName() const { 
  return std::string();
}

///crui::InputMethod* GetHWNDMessageDelegateInputMethod() { }
bool NativeWidgetWin::HasNonClientView() const { 
  return has_non_client_view_;
}

FrameMode NativeWidgetWin::GetFrameMode() const {
  return GetWidget()->ShouldUseNativeFrame() ? FrameMode::SYSTEM_DRAWN
                                             : FrameMode::CUSTOM_DRAWN;
}

bool NativeWidgetWin::HasFrame() const { 
  return !remove_standard_frame_;
}

void NativeWidgetWin::SchedulePaint() { 
  ///GetWidget()->GetRootView()->SchedulePaint();
}

bool NativeWidgetWin::ShouldPaintAsActive() const { 
  return GetWidget()->ShouldPaintAsActive();
}

bool NativeWidgetWin::CanResize() const { 
  return GetWidget()->widget_delegate()->CanResize();
}

bool NativeWidgetWin::CanMaximize() const { 
  return GetWidget()->widget_delegate()->CanMaximize();
}

bool NativeWidgetWin::CanMinimize() const { 
  return GetWidget()->widget_delegate()->CanMinimize();
}

bool NativeWidgetWin::CanActivate() const { 
  ///if (IsModalWindowActive())
  ///  return true;
  return native_widget_delegate_->CanActivate();
}

bool NativeWidgetWin::WantsMouseEventsWhenInactive() const { 
  return wants_mouse_events_when_inactive_;
}

bool NativeWidgetWin::WidgetSizeIsClientSize() const {
  const Widget* widget = GetWidget()->GetTopLevelWidget();
  return IsMaximized() || (widget && widget->ShouldUseNativeFrame());
}

bool NativeWidgetWin::IsModal() const {
  return native_widget_delegate_->IsModal();
}

int NativeWidgetWin::GetInitialShowState() const {
  return CanActivate() ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE;
}

bool NativeWidgetWin::WillProcessWorkAreaChange() const {
  return GetWidget()->widget_delegate()->WillProcessWorkAreaChange();
}

int NativeWidgetWin::GetNonClientComponent(const gfx::Point& point) const {
  gfx::Point dip_position = 
      display::win::ScreenWin::ClientToDIPPoint(GetHWND(), point);
  return native_widget_delegate_->GetNonClientComponent(dip_position);
}

///void GetWindowMask(const gfx::Size& size, SkPath* path) { 
///}

bool NativeWidgetWin::GetClientAreaInsets(gfx::Insets* insets,
                                          HMONITOR monitor) const {
  return false;
}

bool NativeWidgetWin::GetDwmFrameInsetsInPixels(gfx::Insets* insets) const {
  return false;
}

void NativeWidgetWin::GetMinMaxSize(gfx::Size* min_size, 
                                    gfx::Size* max_size) const {
  *min_size = native_widget_delegate_->GetMinimumSize();
  *max_size = native_widget_delegate_->GetMaximumSize();
}

gfx::Size NativeWidgetWin::GetRootViewSize() const { 
  ///return GetWidget()->GetRootView()->size();
  return gfx::Size();
}

gfx::Size NativeWidgetWin::DIPToScreenSize(const gfx::Size& dip_size) const { 
  return display::win::ScreenWin::DIPToScreenSize(GetHWND(), dip_size);
}

void NativeWidgetWin::ResetWindowControls() { 
  ///if (GetWidget()->non_client_view())
  ///  GetWidget()->non_client_view()->ResetWindowControls();
}

gfx::NativeViewAccessible NativeWidgetWin::GetNativeViewAccessible() { 
  ///return GetWidget()->GetRootView()->GetNativeViewAccessible();
  return nullptr;
}

void NativeWidgetWin::HandleActivationChanged(bool active) { 
  native_widget_delegate_->OnNativeWidgetActivationChanged(active);
}

bool NativeWidgetWin::HandleAppCommand(short command) { 
  // We treat APPCOMMAND ids as an extension of our command namespace, and just
  // let the delegate figure out what to do...
  return GetWidget()->widget_delegate() &&
      GetWidget()->widget_delegate()->ExecuteWindowsCommand(command);
}

void NativeWidgetWin::HandleCancelMode() { 
}

void NativeWidgetWin::HandleCaptureLost() { 
}

void NativeWidgetWin::HandleClose() {
  GetWidget()->Close();
}

bool NativeWidgetWin::HandleCommand(int command) { 
  return GetWidget()->widget_delegate()->ExecuteWindowsCommand(command);
}

void NativeWidgetWin::HandleAccelerator(const crui::Accelerator& accelerator) { 
  ///GetWidget()->GetFocusManager()->ProcessAccelerator(accelerator);
}

void NativeWidgetWin::HandleCreate() { 
  SetNativeWindowProperty(kNativeWidgetKey, this);
  CR_CHECK(this == GetNativeWidgetForNativeView(GetNativeView()));

  native_widget_delegate_->OnNativeWidgetCreated();
}

void NativeWidgetWin::HandleDestroying() { 
  ///drag_drop_client_->OnNativeWidgetDestroying(GetHWND());
  native_widget_delegate_->OnNativeWidgetDestroying();

  // Destroy the compositor before destroying the HWND since shutdown
  // may try to swap to the window.
  ///DestroyCompositor();
}

void NativeWidgetWin::HandleDestroyed() { 
  OnFinalMessage();
}

bool NativeWidgetWin::HandleInitialFocus(crui::WindowShowState show_state) { 
  return GetWidget()->SetInitialFocus(show_state);
}

void NativeWidgetWin::HandleDisplayChange() { 
  GetWidget()->widget_delegate()->OnDisplayChanged();
}

void NativeWidgetWin::HandleBeginWMSizeMove() { 
  native_widget_delegate_->OnNativeWidgetBeginUserBoundsChange();
}

void NativeWidgetWin::HandleEndWMSizeMove() { 
  native_widget_delegate_->OnNativeWidgetEndUserBoundsChange();
}

void NativeWidgetWin::HandleMove() { 
  CheckForMonitorChange();
  native_widget_delegate_->OnNativeWidgetMove();
  ///OnHostMovedInPixels(GetBoundsInPixels().origin());
}

void NativeWidgetWin::HandleWorkAreaChanged() { 
  CheckForMonitorChange();
  GetWidget()->widget_delegate()->OnWorkAreaChanged();
}

void NativeWidgetWin::HandleVisibilityChanging(bool visible) { 
  native_widget_delegate_->OnNativeWidgetVisibilityChanging(visible);
}

void NativeWidgetWin::HandleVisibilityChanged(bool visible) { 
  native_widget_delegate_->OnNativeWidgetVisibilityChanged(visible);
}

void NativeWidgetWin::HandleWindowMinimizedOrRestored(bool restored) {
  // Ignore minimize/restore events that happen before widget initialization is
  // done. If a window is created minimized, and then activated, restoring
  // focus will fail because the root window is not visible, which is exposed by
  // ExtensionWindowCreateTest.AcceptState.
  if (!native_widget_delegate_->IsNativeWidgetInitialized())
    return;

  ///if (restored)
  ///  window()->Show();
  ///else
  ///  window()->Hide();
}

void NativeWidgetWin::HandleClientSizeChanged(const gfx::Size& new_size) { 
}

void NativeWidgetWin::HandleFrameChanged() { 
}

void NativeWidgetWin::HandleNativeFocus(HWND last_focused_window) { 
}

void NativeWidgetWin::HandleNativeBlur(HWND focused_window) { 
}

bool NativeWidgetWin::HandleMouseEvent(crui::MouseEvent* event) { 
  native_widget_delegate_->OnMouseEvent(event);
  return event->handled();
}

void NativeWidgetWin::HandleKeyEvent(crui::KeyEvent* event) { 
  // Bypass normal handling of alt-space, which would otherwise consume the
  // corresponding WM_SYSCHAR.  This allows HandleIMEMessage() to show the
  // system menu in this case.  If we instead showed the system menu here, the
  // WM_SYSCHAR would trigger a beep when processed by the native event handler.
  ///if ((event->type() == crui::ET_KEY_PRESSED) &&
  ///    (event->key_code() == crui::VKEY_SPACE) &&
  ///    (event->flags() & crui::EF_ALT_DOWN) &&
  ///    !(event->flags() & crui::EF_CONTROL_DOWN) &&
  ///    GetWidget()->non_client_view()) {
  ///  return;
  ///}
  native_widget_delegate_->OnKeyEvent(event);
}

void NativeWidgetWin::HandleTouchEvent(crui::TouchEvent* event) { 
   // HWNDMessageHandler asynchronously processes touch events. Because of this
  // it's possible for the aura::WindowEventDispatcher to have been destroyed
  // by the time we attempt to process them.
  if (!GetWidget()->GetNativeView())
    return;
}

bool NativeWidgetWin::HandleIMEMessage(UINT message,
                                       WPARAM w_param,
                                       LPARAM l_param,
                                       LRESULT* result) { 
  // Show the system menu at an appropriate location on alt-space.
  ///if ((message == WM_SYSCHAR) && (w_param == VK_SPACE) &&
  ///  GetWidget()->non_client_view()) {
  ///  const auto* frame = GetWidget()->non_client_view()->frame_view();
  ///  ShowSystemMenuAtScreenPixelLocation(
  ///    GetHWND(), frame->GetSystemMenuScreenPixelLocation());
  ///  return true;
  ///}
  ///
  ///MSG msg = {};
  ///msg.hwnd = GetHWND();
  ///msg.message = message;
  ///msg.wParam = w_param;
  ///msg.lParam = l_param;
  ///return GetInputMethod()->OnUntranslatedIMEMessage(msg, result);
  return false;
}

void NativeWidgetWin::HandleInputLanguageChange(DWORD character_set,
                                                HKL input_language_id) { 
  ///GetInputMethod()->OnInputLocaleChanged();
}

void NativeWidgetWin::HandlePaintAccelerated(const gfx::Rect& invalid_rect) {
  ///if (compositor())
  ///  compositor()->ScheduleRedrawRect(invalid_rect);
}

bool NativeWidgetWin::HandleTooltipNotify(int w_param,
                                          NMHDR* l_param,
                                          LRESULT* l_result) { 
  ///return tooltip_ && tooltip_->HandleNotify(w_param, l_param, l_result);
  return false;
}

void NativeWidgetWin::HandleMenuLoop(bool in_menu_loop) { 
  ///if (in_menu_loop) {
  ///  tooltip_disabler_ = std::make_unique<wm::ScopedTooltipDisabler>(window());
  ///} else {
  ///  tooltip_disabler_.reset();
  ///}
}

bool NativeWidgetWin::PreHandleMSG(UINT message,
                                   WPARAM w_param,
                                   LPARAM l_param,
                                   LRESULT* result) {
  return false;
}

void NativeWidgetWin::PostHandleMSG(UINT message, WPARAM w_param, LPARAM l_param) { 
}

bool NativeWidgetWin::HandleScrollEvent(crui::ScrollEvent* event) { 
  native_widget_delegate_->OnMouseEvent(event);
  return event->handled();
}

///bool HandleGestureEvent(crui::GestureEvent* event) { }
void NativeWidgetWin::HandleWindowSizeChanging() { 
  ///if (compositor())
  ///  compositor()->DisableSwapUntilResize();
}

void NativeWidgetWin::HandleWindowSizeUnchanged() { 
  // A resize may not have occurred if the window size happened not to have
  // changed (can occur on Windows 10 when snapping a window to the side of
  // the screen). In that case do a resize to the current size to reenable
  // swaps.
  ///if (compositor())
  ///  compositor()->ReenableSwap();
}

void NativeWidgetWin::HandleWindowScaleFactorChanged(
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

///Widget* NativeWidgetWin::GetWidget() {
///  return native_widget_delegate_->AsWidget();
///}

///const Widget* NativeWidgetWin::GetWidget() const {
///  return native_widget_delegate_->AsWidget();
///}

const Widget* NativeWidgetWin::GetWidgetImpl() const {
  return native_widget_delegate_->AsWidget();
}

HWND NativeWidgetWin::GetHWND() const {
  return message_handler_->hwnd();
}

gfx::Rect NativeWidgetWin::GetBoundsInPixels() const {
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

void NativeWidgetWin::CheckForMonitorChange() {
  HMONITOR monitor_from_window =
      ::MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTOPRIMARY);
  if (monitor_from_window == last_monitor_from_window_)
    return;
  last_monitor_from_window_ = monitor_from_window;
  OnHostDisplayChanged();
}

void NativeWidgetWin::OnHostDisplayChanged() {
  ///if (!compositor())
  ///  return;
  ///display::Display display =
  ///  display::Screen::GetScreen()->GetDisplayNearestWindow(window());
  ///compositor()->SetDisplayColorSpaces(display.color_spaces());
}

void NativeWidgetWin::OnFinalMessage() {
  // We don't destroy props in WM_DESTROY as we may still get messages after
  // WM_DESTROY that assume the properties are still valid (such as WM_CLOSE).
  props_.clear();

  native_widget_delegate_->OnNativeWidgetDestroyed();
  if (ownership_ == Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET)
    delete this;
}

}  // namespace view
}  // namesapce crui
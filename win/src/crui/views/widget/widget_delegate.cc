// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/widget_delegate.h"

#include "crbase/logging.h"
#include "crbase/strings/utf_string_conversions.h"
///#include "ui/accessibility/ax_enums.mojom.h"
#include "crui/display/display.h"
#include "crui/display/screen.h"
///#include "crui/gfx/image/image_skia.h"
#include "crui/views/view.h"
#include "crui/views/views_delegate.h"
#include "crui/views/widget/widget.h"
#include "crui/views/window/client_view.h"

namespace crui {
namespace views {

////////////////////////////////////////////////////////////////////////////////
// WidgetDelegate:

WidgetDelegate::WidgetDelegate() = default;
WidgetDelegate::~WidgetDelegate() {
  CR_CHECK(can_delete_this_) << "A WidgetDelegate must outlive its Widget";
}

void WidgetDelegate::SetCanActivate(bool can_activate) {
  can_activate_ = can_activate;
}

void WidgetDelegate::OnWidgetMove() {
}

void WidgetDelegate::OnDisplayChanged() {
}

void WidgetDelegate::OnWorkAreaChanged() {
}

bool WidgetDelegate::OnCloseRequested(Widget::ClosedReason close_reason) {
  return true;
}

void WidgetDelegate::OnPaintAsActiveChanged(bool paint_as_active) {}

View* WidgetDelegate::GetInitiallyFocusedView() {
  return nullptr;
}

///BubbleDialogDelegateView* WidgetDelegate::AsBubbleDialogDelegate() {
///  return nullptr;
///}

///DialogDelegate* WidgetDelegate::AsDialogDelegate() {
///  return nullptr;
///}

bool WidgetDelegate::CanResize() const {
  return false;
}

bool WidgetDelegate::CanMaximize() const {
  return false;
}

bool WidgetDelegate::CanMinimize() const {
  return false;
}

bool WidgetDelegate::CanActivate() const {
  return can_activate_;
}

crui::ModalType WidgetDelegate::GetModalType() const {
  return crui::MODAL_TYPE_NONE;
}

///ax::mojom::Role WidgetDelegate::GetAccessibleWindowRole() {
///  return ax::mojom::Role::kWindow;
///}

cr::string16 WidgetDelegate::GetAccessibleWindowTitle() const {
  return GetWindowTitle();
}

cr::string16 WidgetDelegate::GetWindowTitle() const {
  return cr::string16();
}

bool WidgetDelegate::ShouldShowWindowTitle() const {
  return true;
}

bool WidgetDelegate::ShouldCenterWindowTitleText() const {
  return false;
}

bool WidgetDelegate::ShouldShowCloseButton() const {
  return true;
}

///gfx::ImageSkia WidgetDelegate::GetWindowAppIcon() {
///  // Use the window icon as app icon by default.
///  return GetWindowIcon();
///}

// Returns the icon to be displayed in the window.
///gfx::ImageSkia WidgetDelegate::GetWindowIcon() {
///  return gfx::ImageSkia();
///}

bool WidgetDelegate::ShouldShowWindowIcon() const {
  return false;
}

bool WidgetDelegate::ExecuteWindowsCommand(int command_id) {
  return false;
}

std::string WidgetDelegate::GetWindowName() const {
  return std::string();
}

void WidgetDelegate::SaveWindowPlacement(const gfx::Rect& bounds,
                                         crui::WindowShowState show_state) {
  std::string window_name = GetWindowName();
  if (!window_name.empty()) {
    ViewsDelegate::GetInstance()->SaveWindowPlacement(GetWidget(), window_name,
                                                      bounds, show_state);
  }
}

bool WidgetDelegate::GetSavedWindowPlacement(
    const Widget* widget,
    gfx::Rect* bounds,
    crui::WindowShowState* show_state) const {
  std::string window_name = GetWindowName();
  if (window_name.empty()/* ||
      !ViewsDelegate::GetInstance()->GetSavedWindowPlacement(
          widget, window_name, bounds, show_state)*/)
    return false;
  // Try to find a display intersecting the saved bounds.
  const auto& display =
      display::Screen::GetScreen()->GetDisplayMatching(*bounds);
  return display.bounds().Intersects(*bounds);
}

bool WidgetDelegate::ShouldRestoreWindowSize() const {
  return true;
}

View* WidgetDelegate::GetContentsView() {
  if (!default_contents_view_)
    default_contents_view_ = new View;
  return default_contents_view_;
}

ClientView* WidgetDelegate::CreateClientView(Widget* widget) {
  return new ClientView(widget, GetContentsView());
}

NonClientFrameView* WidgetDelegate::CreateNonClientFrameView(Widget* widget) {
  return nullptr;
}

View* WidgetDelegate::CreateOverlayView() {
  return nullptr;
}

bool WidgetDelegate::WillProcessWorkAreaChange() const {
  return false;
}

bool WidgetDelegate::WidgetHasHitTestMask() const {
  return false;
}

///void WidgetDelegate::GetWidgetHitTestMask(SkPath* mask) const {
///  CR_DCHECK(mask);
///}

bool WidgetDelegate::ShouldAdvanceFocusToTopLevelWidget() const {
  return false;
}

bool WidgetDelegate::ShouldDescendIntoChildForEventHandling(
    gfx::NativeView child,
    const gfx::Point& location) {
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// WidgetDelegateView:


WidgetDelegateView::WidgetDelegateView() {
  // A WidgetDelegate should be deleted on DeleteDelegate.
  set_owned_by_client();
}

WidgetDelegateView::~WidgetDelegateView() = default;

void WidgetDelegateView::DeleteDelegate() {
  delete this;
}

views::View* WidgetDelegateView::GetContentsView() {
  return this;
}

const Widget* WidgetDelegateView::GetWidgetImpl() const {
  return View::GetWidgetImpl();
}

BEGIN_METADATA(WidgetDelegateView)
METADATA_PARENT_CLASS(View)
END_METADATA()

}  // namespace views
}  // namespace crui

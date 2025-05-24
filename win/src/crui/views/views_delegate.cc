// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/views_delegate.h"

#include <utility>

//#include "crbase/command_line.h"
///#include "crbuild/build_config.h"
///#include "crui/views/views_touch_selection_controller_factory.h"
#include "crui/views/widget/native_widget_private.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_USE_AURA)
///#include "crui/views/touchui/touch_selection_menu_runner_views.h"
#endif

namespace crui {
namespace views {

namespace {

ViewsDelegate* views_delegate = nullptr;

}  // namespace

ViewsDelegate::ViewsDelegate()
    /*: editing_controller_factory_(new ViewsTouchEditingControllerFactory) */{
  CR_DCHECK(!views_delegate);
  views_delegate = this;

  ///crui::TouchEditingControllerFactory::SetInstance(
  ///    editing_controller_factory_.get());

#if defined(MINI_CHROMIUM_USE_AURA)
  ///touch_selection_menu_runner_ =
  ///    std::make_unique<TouchSelectionMenuRunnerViews>();
#endif
}

ViewsDelegate::~ViewsDelegate() {
  ///crui::TouchEditingControllerFactory::SetInstance(nullptr);

  CR_DCHECK(this == views_delegate);
  views_delegate = nullptr;
}

ViewsDelegate* ViewsDelegate::GetInstance() {
  return views_delegate;
}

void ViewsDelegate::SaveWindowPlacement(const Widget* widget,
                                        const std::string& window_name,
                                        const gfx::Rect& bounds,
                                        crui::WindowShowState show_state) {}

bool ViewsDelegate::GetSavedWindowPlacement(
    const Widget* widget,
    const std::string& window_name,
    gfx::Rect* bounds,
    crui::WindowShowState* show_state) const {
  return false;
}

void ViewsDelegate::NotifyMenuItemFocused(const cr::string16& menu_name,
                                          const cr::string16& menu_item_name,
                                          int item_index,
                                          int item_count,
                                          bool has_submenu) {}

///ViewsDelegate::ProcessMenuAcceleratorResult
///ViewsDelegate::ProcessAcceleratorWhileMenuShowing(
///    const crui::Accelerator& accelerator) {
///  return ProcessMenuAcceleratorResult::LEAVE_MENU_OPEN;
///}

#if defined(MINI_CHROMIUM_OS_WIN)
HICON ViewsDelegate::GetDefaultWindowIcon() const {
  return nullptr;
}

HICON ViewsDelegate::GetSmallWindowIcon() const {
  return nullptr;
}

bool ViewsDelegate::IsWindowInMetro(gfx::NativeWindow window) const {
  return false;
}
#elif defined(MINI_CHROMIUM_OS_LINUX) && \
      defined(MINI_CHROMIUM_ENABLE_DESKTOP_AURA)
gfx::ImageSkia* ViewsDelegate::GetDefaultWindowIcon() const {
  return nullptr;
}
#endif

NonClientFrameView* ViewsDelegate::CreateDefaultNonClientFrameView(
    Widget* widget) {
  return nullptr;
}

bool ViewsDelegate::IsShuttingDown() const {
  return false;
}

void ViewsDelegate::AddRef() {}

void ViewsDelegate::ReleaseRef() {}

void ViewsDelegate::OnBeforeWidgetInit(
    Widget::InitParams* params,
    internal::NativeWidgetDelegate* delegate) {}

bool ViewsDelegate::WindowManagerProvidesTitleBar(bool maximized) {
  return false;
}

///crui::ContextFactory* ViewsDelegate::GetContextFactory() {
///  return nullptr;
///}

///ui::ContextFactoryPrivate* ViewsDelegate::GetContextFactoryPrivate() {
///  return nullptr;
///}

std::string ViewsDelegate::GetApplicationName() {
  ///cr::FilePath program = base::CommandLine::ForCurrentProcess()->GetProgram();
  ///return program.BaseName().AsUTF8Unsafe();
  return "ggchromium";
}

#if defined(MINI_CHROMIUM_OS_WIN)
int ViewsDelegate::GetAppbarAutohideEdges(HMONITOR monitor,
                                          cr::OnceClosure callback) {
  return EDGE_BOTTOM;
}
#endif

#if defined(MINI_CHROMIUM_USE_AURA)
///void ViewsDelegate::SetTouchSelectionMenuRunner(
///    std::unique_ptr<TouchSelectionMenuRunnerViews> menu_runner) {
///  touch_selection_menu_runner_ = std::move(menu_runner);
///}
#endif

}  // namespace views
}  // namespace crui

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_VIEWS_DELEGATE_H_
#define UI_VIEWS_VIEWS_DELEGATE_H_

#include <memory>
#include <string>
#include <utility>

#include "crbase/functional/callback.h"
#include "crbase/location.h"
#include "crbase/strings/string16.h"
#include "crui/base/ui_base_types.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/views/widget/widget.h"
#include "crui/base/ui_export.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#endif

namespace crui {

///class ContextFactory;
///class TouchEditingControllerFactory;
class GestureRecognizer;

namespace gfx {
///class ImageSkia;
class Rect;
}  // namespace gfx

namespace views {

class NativeWidget;
///class NonClientFrameView;
class Widget;

#if defined(MINI_CHROMIUM_USE_AURA)
class TouchSelectionMenuRunnerViews;
#endif

namespace internal {
class NativeWidgetDelegate;
}  // namespace internal

// ViewsDelegate is an interface implemented by an object using the views
// framework. It is used to obtain various high level application utilities
// and perform some actions such as window placement saving.
//
// The embedding app must set the ViewsDelegate instance by instantiating an
// implementation of ViewsDelegate (the constructor will set the instance).
class CRUI_EXPORT ViewsDelegate {
 public:
  using NativeWidgetFactory =
      cr::RepeatingCallback<NativeWidget*(const Widget::InitParams&,
                                          internal::NativeWidgetDelegate*)>;
#if defined(MINI_CHROMIUM_OS_WIN)
  enum AppbarAutohideEdge {
    EDGE_TOP    = 1 << 0,
    EDGE_LEFT   = 1 << 1,
    EDGE_BOTTOM = 1 << 2,
    EDGE_RIGHT  = 1 << 3,
  };
#endif

  enum class ProcessMenuAcceleratorResult {
    // The accelerator was handled while the menu was showing. No further action
    // is needed and the menu should be kept open.
    LEAVE_MENU_OPEN,

    // The accelerator was not handled. The menu should be closed and event
    // handling should stop for this event.
    CLOSE_MENU,
  };

  ViewsDelegate(const ViewsDelegate&) = delete;
  ViewsDelegate& operator=(const ViewsDelegate&) = delete;

  virtual ~ViewsDelegate();

  // Returns the ViewsDelegate instance.  This should never return non-null
  // unless the binary has not yet initialized the delegate, so callers should
  // not generally null-check.
  static ViewsDelegate* GetInstance();

  // Call this method to set a factory callback that will be used to construct
  // NativeWidget implementations overriding the platform defaults.
  void set_native_widget_factory(NativeWidgetFactory factory) {
    native_widget_factory_ = std::move(factory);
  }
  const NativeWidgetFactory& native_widget_factory() const {
    return native_widget_factory_;
  }

  void set_gesture_recognizer(std::unique_ptr<GestureRecognizer> gr);
  GestureRecognizer* gesture_recognizer() {
    return gesture_recognizer_.get();
  }

  // Saves the position, size and "show" state for the window with the
  // specified name.
  virtual void SaveWindowPlacement(const Widget* widget,
                                   const std::string& window_name,
                                   const gfx::Rect& bounds,
                                   crui::WindowShowState show_state);

  // Retrieves the saved position and size and "show" state for the window with
  // the specified name.
  virtual bool GetSavedWindowPlacement(const Widget* widget,
                                       const std::string& window_name,
                                       gfx::Rect* bounds,
                                       crui::WindowShowState* show_state) const;

  // For accessibility, notify the delegate that a menu item was focused
  // so that alternate feedback (speech / magnified text) can be provided.
  virtual void NotifyMenuItemFocused(const cr::string16& menu_name,
                                     const cr::string16& menu_item_name,
                                     int item_index,
                                     int item_count,
                                     bool has_submenu);

  // If |accelerator| can be processed while a menu is showing, it will be
  // processed now and LEAVE_MENU_OPEN is returned. Otherwise, |accelerator|
  // will be reposted for processing later after the menu closes and CLOSE_MENU
  // will be returned.
  ///virtual ProcessMenuAcceleratorResult ProcessAcceleratorWhileMenuShowing(
  ///    const crui::Accelerator& accelerator);

#if defined(MINI_CHROMIUM_OS_WIN)
  // Retrieves the default window icon to use for windows if none is specified.
  virtual HICON GetDefaultWindowIcon() const;
  // Retrieves the small window icon to use for windows if none is specified.
  virtual HICON GetSmallWindowIcon() const;
  // Returns true if the window passed in is in the Windows 8 metro
  // environment.
  virtual bool IsWindowInMetro(gfx::NativeWindow window) const;
#elif defined(MINI_CHROMIUM_OS_LINUX) && BUILDFLAG(ENABLE_DESKTOP_AURA)
  virtual gfx::ImageSkia* GetDefaultWindowIcon() const;
#endif

  // Creates a default NonClientFrameView to be used for windows that don't
  // specify their own. If this function returns NULL, the
  // views::CustomFrameView type will be used.
  virtual NonClientFrameView* CreateDefaultNonClientFrameView(Widget* widget);

  // AddRef/ReleaseRef are invoked while a menu is visible. They are used to
  // ensure we don't attempt to exit while a menu is showing.
  virtual void AddRef();
  virtual void ReleaseRef();
  // Returns true if the application is shutting down. AddRef/Release should not
  // be called in this situation.
  virtual bool IsShuttingDown() const;

  // Gives the platform a chance to modify the properties of a Widget.
  virtual void OnBeforeWidgetInit(Widget::InitParams* params,
                                  internal::NativeWidgetDelegate* delegate);

  // Returns true if the operating system's window manager will always provide a
  // title bar with caption buttons (ignoring the setting to
  // |remove_standard_frame| in InitParams). If |maximized|, this applies to
  // maximized windows; otherwise to restored windows.
  virtual bool WindowManagerProvidesTitleBar(bool maximized);

  // Returns the context factory for new windows.
  ///virtual crui::ContextFactory* GetContextFactory();

  // Returns the privileged context factory for new windows.
  ///virtual crui::ContextFactoryPrivate* GetContextFactoryPrivate();

  // Returns the user-visible name of the application.
  virtual std::string GetApplicationName();

#if defined(MINI_CHROMIUM_OS_WIN)
  // Starts a query for the appbar autohide edges of the specified monitor and
  // returns the current value.  If the query finds the edges have changed from
  // the current value, |callback| is subsequently invoked.  If the edges have
  // not changed, |callback| is never run.
  //
  // The return value is a bitmask of AppbarAutohideEdge.
  virtual int GetAppbarAutohideEdges(HMONITOR monitor,
                                     cr::OnceClosure callback);
#endif

 protected:
  ViewsDelegate();

#if defined(MINI_CHROMIUM_USE_AURA)
  void SetTouchSelectionMenuRunner(
      std::unique_ptr<TouchSelectionMenuRunnerViews> menu_runner);
#endif

 private:
  ///std::unique_ptr<crui::TouchEditingControllerFactory>
  ///    editing_controller_factory_;

#if defined(MINI_CHROMIUM_USE_AURA)
  std::unique_ptr<TouchSelectionMenuRunnerViews> touch_selection_menu_runner_;
#endif

  std::unique_ptr<GestureRecognizer> gesture_recognizer_; // from aura/env.h
  NativeWidgetFactory native_widget_factory_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_VIEWS_DELEGATE_H_

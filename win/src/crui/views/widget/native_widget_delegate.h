// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_NATIVE_WIDGET_DELEGATE_H_
#define UI_VIEWS_WIDGET_NATIVE_WIDGET_DELEGATE_H_

#include "crui/events/event_constants.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_base_types.h"
#include "crui/base/ui_export.h"

///class SkPath;

namespace crui {

namespace gfx {
class Canvas;
class Point;
class Size;
}  // namespace gfx

class GestureEvent;
class KeyEvent;
class MouseEvent;
class ScrollEvent;
///class Layer;
///class PaintContext;

namespace views {
class Widget;

namespace internal {

////////////////////////////////////////////////////////////////////////////////
// NativeWidgetDelegate
//
//  An interface implemented by the object that handles events sent by a
//  NativeWidget implementation.
//
class CRUI_EXPORT NativeWidgetDelegate {
 public:
  virtual ~NativeWidgetDelegate() = default;

  // Returns true if the window is modal.
  virtual bool IsModal() const = 0;

  // Returns true if the window is a dialog box.
  virtual bool IsDialogBox() const = 0;

  // Returns true if the window can be activated.
  virtual bool CanActivate() const = 0;

  // Returns true if the window should paint as active.
  virtual bool ShouldPaintAsActive() const = 0;

  // Returns true if the native widget has been initialized.
  virtual bool IsNativeWidgetInitialized() const = 0;

  // Called when the activation state of a window has changed.
  // Returns true if this event should be handled.
  virtual bool OnNativeWidgetActivationChanged(bool active) = 0;

  // Called when native focus moves from one native view to another.
  virtual void OnNativeFocus() = 0;
  virtual void OnNativeBlur() = 0;

  // Called when the window is about to be shown/hidden.
  virtual void OnNativeWidgetVisibilityChanging(bool visible) = 0;

  // Called when the window is shown/hidden.
  virtual void OnNativeWidgetVisibilityChanged(bool visible) = 0;

  // Called when the native widget is created.
  virtual void OnNativeWidgetCreated() = 0;

  // Called just before the native widget is destroyed. This is the delegate's
  // last chance to do anything with the native widget handle.
  virtual void OnNativeWidgetDestroying() = 0;

  // Called just after the native widget is destroyed.
  virtual void OnNativeWidgetDestroyed() = 0;

  // Returns the smallest size the window can be resized to by the user.
  virtual gfx::Size GetMinimumSize() const = 0;

  // Returns the largest size the window can be resized to by the user.
  virtual gfx::Size GetMaximumSize() const = 0;

  // Called when the NativeWidget changed position.
  virtual void OnNativeWidgetMove() = 0;

  // Called when the NativeWidget changed size to |new_size|.
  // This may happen at the same time as OnNativeWidgetWindowShowStateChanged,
  // e.g. maximize.
  virtual void OnNativeWidgetSizeChanged(const gfx::Size& new_size) = 0;

  // Called when NativeWidget changed workspaces.
  virtual void OnNativeWidgetWorkspaceChanged() = 0;

  // Called when the NativeWidget changes its window state.
  // This may happen at the same time as OnNativeWidgetSizeChanged, e.g.
  // maximize.
  virtual void OnNativeWidgetWindowShowStateChanged() = 0;

  // Called when the user begins/ends to change the bounds of the window.
  virtual void OnNativeWidgetBeginUserBoundsChange() = 0;
  virtual void OnNativeWidgetEndUserBoundsChange() = 0;

  // Returns true if the delegate has a FocusManager.
  virtual bool HasFocusManager() const = 0;

  // Paints the rootview in the context. This will also refresh the compositor
  // tree if necessary.
  virtual void OnNativeWidgetPaint(gfx::Canvas* canvas) = 0;

  // Returns the non-client component (see ui/base/hit_test.h) containing
  // |point|, in client coordinates.
  virtual int GetNonClientComponent(const gfx::Point& point) = 0;

  // Event handlers.
  virtual void OnKeyEvent(crui::KeyEvent* event) = 0;
  virtual void OnMouseEvent(crui::MouseEvent* event) = 0;
  virtual void OnMouseCaptureLost() = 0;
  virtual void OnScrollEvent(crui::ScrollEvent* event) = 0;
  virtual void OnGestureEvent(crui::GestureEvent* event) = 0;

  // Runs the specified native command. Returns true if the command is handled.
  virtual bool ExecuteCommand(int command_id) = 0;

  // Returns true if window has a hit-test mask.
  virtual bool HasHitTestMask() const = 0;

  // Provides the hit-test mask if HasHitTestMask above returns true.
  ///virtual void GetHitTestMask(SkPath* mask) const = 0;

  virtual Widget* AsWidget() = 0;
  virtual const Widget* AsWidget() const = 0;

  // Sets-up the focus manager with the view that should have focus when the
  // window is shown the first time.  It takes the intended |show_state| of the
  // window in order to decide whether the window should be focused now or
  // later.  Returns true if the initial focus has been set or the window should
  // not set the initial focus, or false if the caller should set the initial
  // focus (if any).
  virtual bool SetInitialFocus(crui::WindowShowState show_state) = 0;

  // Returns true if event handling should descend into |child|. |root_layer| is
  // the layer associated with the root Window and |child_layer| the layer
  // associated with |child|. |location| is in terms of the Window.
  ///virtual bool ShouldDescendIntoChildForEventHandling(
  ///    ui::Layer* root_layer,
  ///    gfx::NativeView child,
  ///    ui::Layer* child_layer,
  ///    const gfx::Point& location) = 0;

  // Called to process a previous call to ScheduleLayout().
  virtual void LayoutRootViewIfNecessary() = 0;
};

}  // namespace internal
}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_NATIVE_WIDGET_DELEGATE_H_

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_NATIVE_WIDGET_AURA_H_
#define UI_VIEWS_WIDGET_NATIVE_WIDGET_AURA_H_

#include <string>

#include "crbase/memory/weak_ptr.h"
#include "crui/aura/client/drag_drop_delegate.h"
#include "crui/aura/client/focus_change_observer.h"
#include "crui/aura/window_delegate.h"
#include "crui/aura/window_observer.h"
#include "crui/base/cursor/cursor.h"
#include "crui/events/event_constants.h"
#include "crui/views/widget/native_widget_private.h"
#include "crui/aura/wm/public/activation_change_observer.h"
#include "crui/aura/wm/public/activation_delegate.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_MACOSX)
#error This file must not be included on macOS; Chromium Mac doesn't use Aura.
#endif

namespace crui {

namespace aura {
class Window;
}  // namespace aura

namespace views {

class DropHelper;
class FocusManagerEventHandler;
class TooltipManagerAura;
class WindowReorderer;

class CRUI_EXPORT NativeWidgetAura : public internal::NativeWidgetPrivate,
                                     public aura::WindowDelegate,
                                     public aura::WindowObserver,
                                     public wm::ActivationDelegate,
                                     public wm::ActivationChangeObserver,
                                     public aura::client::FocusChangeObserver,
                                     public aura::client::DragDropDelegate {
 public:
  NativeWidgetAura(const NativeWidgetAura&) = delete;
  NativeWidgetAura& operator=(const NativeWidgetAura&) = delete;

  explicit NativeWidgetAura(internal::NativeWidgetDelegate* delegate);

  // Called internally by NativeWidgetAura and DesktopNativeWidgetAura to
  // associate |native_widget| with |window|.
  static void RegisterNativeWidgetForWindow(
      internal::NativeWidgetPrivate* native_widget,
      aura::Window* window);

  // Assign an icon to aura window.
  ///static void AssignIconToAuraWindow(aura::Window* window,
  ///                                   const gfx::ImageSkia& window_icon,
  ///                                   const gfx::ImageSkia& app_icon);

  // If necessary, sets the ShadowElevation of |window| from |params|.
  static void SetShadowElevationFromInitParams(
      aura::Window* window,
      const Widget::InitParams& params);

  // Sets the window property aura::client::kResizeBehaviorKey based on the
  // values from the delegate.
  static void SetResizeBehaviorFromDelegate(WidgetDelegate* delegate,
                                            aura::Window* window);

  // internal::NativeWidgetPrivate:
  void InitNativeWidget(Widget::InitParams params) override;
  void OnWidgetInitDone() override;
  NonClientFrameView* CreateNonClientFrameView() override;
  bool ShouldUseNativeFrame() const override;
  bool ShouldWindowContentsBeTransparent() const override;
  void FrameTypeChanged() override;
  gfx::NativeView GetNativeView() const override;
  gfx::NativeWindow GetNativeWindow() const override;
  Widget* GetTopLevelWidget() override;
  ///const ui::Compositor* GetCompositor() const override;
  const crui::Layer* GetLayer() const override;
  void ReorderNativeViews() override;
  void ViewRemoved(View* view) override;
  void SetNativeWindowProperty(const char* name, void* value) override;
  void* GetNativeWindowProperty(const char* name) const override;
  TooltipManager* GetTooltipManager() const override;
  void SetCapture() override;
  void ReleaseCapture() override;
  bool HasCapture() const override;
  ///crui::InputMethod* GetInputMethod() override;
  void CenterWindow(const gfx::Size& size) override;
  void GetWindowPlacement(gfx::Rect* bounds,
                          crui::WindowShowState* maximized) const override;
  bool SetWindowTitle(const cr::string16& title) override;
  ///void SetWindowIcons(const gfx::ImageSkia& window_icon,
  ///                    const gfx::ImageSkia& app_icon) override;
  void InitModalType(crui::ModalType modal_type) override;
  gfx::Rect GetWindowBoundsInScreen() const override;
  gfx::Rect GetClientAreaBoundsInScreen() const override;
  gfx::Rect GetRestoredBounds() const override;
  std::string GetWorkspace() const override;
  void SetBounds(const gfx::Rect& bounds) override;
  void SetBoundsConstrained(const gfx::Rect& bounds) override;
  void SetSize(const gfx::Size& size) override;
  void StackAbove(gfx::NativeView native_view) override;
  void StackAtTop() override;
  void SetShape(std::unique_ptr<Widget::ShapeRects> shape) override;
  void Close() override;
  void CloseNow() override;
  void Show(crui::WindowShowState show_state,
            const gfx::Rect& restore_bounds) override;
  void Hide() override;
  bool IsVisible() const override;
  void Activate() override;
  void Deactivate() override;
  bool IsActive() const override;
  void SetZOrderLevel(crui::ZOrderLevel order) override;
  crui::ZOrderLevel GetZOrderLevel() const override;
  void SetVisibleOnAllWorkspaces(bool always_visible) override;
  bool IsVisibleOnAllWorkspaces() const override;
  void Maximize() override;
  void Minimize() override;
  bool IsMaximized() const override;
  bool IsMinimized() const override;
  void Restore() override;
  void SetFullscreen(bool fullscreen) override;
  bool IsFullscreen() const override;
  void SetCanAppearInExistingFullscreenSpaces(
      bool can_appear_in_existing_fullscreen_spaces) override;
  void SetOpacity(float opacity) override;
  void SetAspectRatio(const gfx::SizeF& aspect_ratio) override;
  void FlashFrame(bool flash_frame) override;
  void RunShellDrag(View* view,
                    std::unique_ptr<crui::OSExchangeData> data,
                    const gfx::Point& location,
                    int operation,
                    crui::DragDropTypes::DragEventSource source) override;
  void SchedulePaintInRect(const gfx::Rect& rect) override;
  void ScheduleLayout() override;
  void SetCursor(gfx::NativeCursor cursor) override;
  bool IsMouseEventsEnabled() const override;
  bool IsMouseButtonDown() const override;
  void ClearNativeFocus() override;
  gfx::Rect GetWorkAreaBoundsInScreen() const override;
  Widget::MoveLoopResult RunMoveLoop(
      const gfx::Vector2d& drag_offset,
      Widget::MoveLoopSource source,
      Widget::MoveLoopEscapeBehavior escape_behavior) override;
  void EndMoveLoop() override;
  void SetVisibilityChangedAnimationsEnabled(bool value) override;
  void SetVisibilityAnimationDuration(const cr::TimeDelta& duration) override;
  void SetVisibilityAnimationTransition(
      Widget::VisibilityTransition transition) override;
  bool IsTranslucentWindowOpacitySupported() const override;
  crui::GestureRecognizer* GetGestureRecognizer() override;
  void OnSizeConstraintsChanged() override;
  void OnNativeViewHierarchyWillChange() override;
  void OnNativeViewHierarchyChanged() override;
  std::string GetName() const override;

  // aura::WindowDelegate:
  gfx::Size GetMinimumSize() const override;
  gfx::Size GetMaximumSize() const override;
  void OnBoundsChanged(const gfx::Rect& old_bounds,
                       const gfx::Rect& new_bounds) override;
  gfx::NativeCursor GetCursor(const gfx::Point& point) override;
  int GetNonClientComponent(const gfx::Point& point) const override;
  bool ShouldDescendIntoChildForEventHandling(
      aura::Window* child,
      const gfx::Point& location) override;
  bool CanFocus() override;
  void OnCaptureLost() override;
  void OnPaint(gfx::Canvas* canvas) override;
  void OnDeviceScaleFactorChanged(float old_device_scale_factor,
                                  float new_device_scale_factor) override;
  void OnWindowDestroying(aura::Window* window) override;
  void OnWindowDestroyed(aura::Window* window) override;
  void OnWindowTargetVisibilityChanged(bool visible) override;
  bool HasHitTestMask() const override;
  ///void GetHitTestMask(SkPath* mask) const override;
  void UpdateVisualState() override;

  // aura::WindowObserver:
  void OnWindowPropertyChanged(aura::Window* window,
                               const void* key,
                               intptr_t old) override;
  void OnResizeLoopStarted(aura::Window* window) override;
  void OnResizeLoopEnded(aura::Window* window) override;

  // ui::EventHandler:
  void OnKeyEvent(crui::KeyEvent* event) override;
  void OnMouseEvent(crui::MouseEvent* event) override;
  void OnScrollEvent(crui::ScrollEvent* event) override;
  void OnGestureEvent(crui::GestureEvent* event) override;

  // wm::ActivationDelegate:
  bool ShouldActivate() const override;

  // wm::ActivationChangeObserver:
  void OnWindowActivated(wm::ActivationChangeObserver::ActivationReason reason,
                         aura::Window* gained_active,
                         aura::Window* lost_active) override;

  // aura::client::FocusChangeObserver:
  void OnWindowFocused(aura::Window* gained_focus,
                       aura::Window* lost_focus) override;

  // aura::client::DragDropDelegate:
  void OnDragEntered(const crui::DropTargetEvent& event) override;
  int OnDragUpdated(const crui::DropTargetEvent& event) override;
  void OnDragExited() override;
  int OnPerformDrop(const crui::DropTargetEvent& event,
                    std::unique_ptr<crui::OSExchangeData> data) override;

 protected:
  ~NativeWidgetAura() override;

  internal::NativeWidgetDelegate* delegate() { return delegate_; }

 private:
  void SetInitialFocus(crui::WindowShowState show_state);

  // internal::NativeWidgetPrivate:
  const Widget* GetWidgetImpl() const override;

  internal::NativeWidgetDelegate* delegate_;

  // WARNING: set to NULL when destroyed. As the Widget is not necessarily
  // destroyed along with |window_| all usage of |window_| should first verify
  // non-NULL.
  aura::Window* window_;

  // See class documentation for Widget in widget.h for a note about ownership.
  Widget::InitParams::Ownership ownership_;

  // Are we in the destructor?
  bool destroying_;

  gfx::NativeCursor cursor_;

  std::unique_ptr<TooltipManagerAura> tooltip_manager_;

  // Reorders child windows of |window_| associated with a view based on the
  // order of the associated views in the widget's view hierarchy.
  std::unique_ptr<WindowReorderer> window_reorderer_;

  std::unique_ptr<DropHelper> drop_helper_;
  int last_drop_operation_;

  // Native widget's handler to receive events before the event target.
  std::unique_ptr<FocusManagerEventHandler> focus_manager_event_handler_;

  // The following factory is used for calls to close the NativeWidgetAura
  // instance.
  cr::WeakPtrFactory<NativeWidgetAura> close_widget_factory_{this};
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_NATIVE_WIDGET_AURA_H_

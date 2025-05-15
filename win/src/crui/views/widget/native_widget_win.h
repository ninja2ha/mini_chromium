// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_NATIVE_WIDGET_WIN_H_
#define UI_VIEWS_WIDGET_NATIVE_WIDGET_WIN_H_

#include <memory>
#include <vector>

#include "crbase/win/win_util.h"
#include "crui/gfx/win/window_impl.h"
#include "crui/views/widget/native_widget_private.h"
#include "crui/views/win/hwnd_message_handler_delegate.h"
#include "crui/base/ui_export.h"

namespace crui {

///class Compositor;
class ViewProp;

namespace gfx {
class Rect;
}  // namespace gfx

namespace views {

class HWNDMessageHandler;

///////////////////////////////////////////////////////////////////////////////
//
// NativeWidgetWin
//  A Widget for a views hierarchy used to represent anything that can be
//  contained within an HWND, e.g. a control, a window, etc. Specializations
//  suitable for specific tasks, e.g. top level window, are derived from this.
//
//  This Widget contains a RootView which owns the hierarchy of views within it.
//  As long as views are part of this tree, they will be deleted automatically
//  when the RootView is destroyed. If you remove a view from the tree, you are
//  then responsible for cleaning up after it.
//
///////////////////////////////////////////////////////////////////////////////
class CRUI_EXPORT NativeWidgetWin : public internal::NativeWidgetPrivate,
                                    public HWNDMessageHandlerDelegate {
 public:
  explicit NativeWidgetWin(internal::NativeWidgetDelegate* delegate);
  virtual ~NativeWidgetWin();

 protected:
  // overriden from internal::NativeWidgetPrivate
  
  // Initializes the NativeWidget.
  virtual void InitNativeWidget(Widget::InitParams params) override;

  // Called at the end of Widget::Init(), after Widget has completed
  // initialization.
  virtual void OnWidgetInitDone() override;

  // Returns a NonClientFrameView for the widget's NonClientView, or NULL if
  // the NativeWidget wants no special NonClientFrameView.
  virtual NonClientFrameView* CreateNonClientFrameView() override;

  virtual bool ShouldUseNativeFrame() const override;
  virtual bool ShouldWindowContentsBeTransparent() const override;
  virtual void FrameTypeChanged() override;

  // Returns the NativeView/Window associated with this NativeWidget.
  virtual gfx::NativeView GetNativeView() const override;
  virtual gfx::NativeWindow GetNativeWindow() const override;

  // Returns the topmost Widget in a hierarchy.
  virtual Widget* GetTopLevelWidget() override;

  // Returns the Compositor, or NULL if there isn't one associated with this
  // NativeWidget.
  ///virtual const ui::Compositor* GetCompositor() const override;

  // Returns the NativeWidget's layer, if any.
  ///virtual const ui::Layer* GetLayer() const override;

  // Reorders the widget's child NativeViews which are associated to the view
  // tree (eg via a NativeViewHost) to match the z-order of the views in the
  // view tree. The z-order of views with layers relative to views with
  // associated NativeViews is used to reorder the NativeView layers. This
  // method assumes that the widget's child layers which are owned by a view are
  // already in the correct z-order relative to each other and does no
  // reordering if there are no views with an associated NativeView.
  virtual void ReorderNativeViews() override;

  // Notifies the NativeWidget that a view was removed from the Widget's view
  // hierarchy.
  virtual void ViewRemoved(View* view) override;

  // Sets/Gets a native window property on the underlying native window object.
  // Returns NULL if the property does not exist. Setting the property value to
  // NULL removes the property.
  virtual void SetNativeWindowProperty(const char* key, void* value) override;
  virtual void* GetNativeWindowProperty(const char* key) const override;

  // Returns the native widget's tooltip manager. Called from the View hierarchy
  // to update tooltips.
  ///virtual TooltipManager* GetTooltipManager() const override;

  // Sets or releases event capturing for this native widget.
  virtual void SetCapture() override;
  virtual void ReleaseCapture() override;

  // Returns true if this native widget is capturing events.
  virtual bool HasCapture() const override;

  // Returns the ui::InputMethod for this native widget.
  ///virtual ui::InputMethod* GetInputMethod() override;

  // Centers the window and sizes it to the specified size.
  virtual void CenterWindow(const gfx::Size& size) override;

  // Retrieves the window's current restored bounds and "show" state, for
  // persisting.
  virtual void GetWindowPlacement(
      gfx::Rect* bounds,
      crui::WindowShowState* show_state) const override;

  // Sets the NativeWindow title. Returns true if the title changed.
  virtual bool SetWindowTitle(const cr::string16& title) override;

  // Sets the Window icons. |window_icon| is a 16x16 icon suitable for use in
  // a title bar. |app_icon| is a larger size for use in the host environment
  // app switching UI.
  ///virtual void SetWindowIcons(const gfx::ImageSkia& window_icon,
  ///                            const gfx::ImageSkia& app_icon) override;

  // Initializes the modal type of the window to |modal_type|. Called from
  // NativeWidgetDelegate::OnNativeWidgetCreated() before the widget is
  // initially parented.
  virtual void InitModalType(crui::ModalType modal_type) override;

  // See method documentation in Widget.
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
  void FlashFrame(bool flash) override;
  ///virtual void RunShellDrag(View* view,
  ///                          std::unique_ptr<ui::OSExchangeData> data,
  ///                          const gfx::Point& location,
  ///                          int operation,
  ///                          ui::DragDropTypes::DragEventSource source) override;
  void SchedulePaintInRect(const gfx::Rect& rect) override;
  void ScheduleLayout() override;
  void SetCursor(gfx::NativeCursor cursor) override;
  ///virtual void ShowEmojiPanel() override;
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
  void SetVisibilityAnimationDuration(
      const cr::TimeDelta& duration) override;
  void SetVisibilityAnimationTransition(
      Widget::VisibilityTransition transition) override;
  bool IsTranslucentWindowOpacitySupported() const override;
  ///virtual crui::GestureRecognizer* GetGestureRecognizer() override;
  void OnSizeConstraintsChanged() override;
  void OnNativeViewHierarchyWillChange() override;
  void OnNativeViewHierarchyChanged() override;
  std::string GetName() const override;

  
  // Overridden from HWNDMessageHandlerDelegate:
  ///crui::InputMethod* GetHWNDMessageDelegateInputMethod() override;
  bool HasNonClientView() const override;
  FrameMode GetFrameMode() const override;
  bool HasFrame() const override;
  void SchedulePaint() override;
  bool ShouldPaintAsActive() const override;
  bool CanResize() const override;
  bool CanMaximize() const override;
  bool CanMinimize() const override;
  bool CanActivate() const override;
  bool WantsMouseEventsWhenInactive() const override;
  bool WidgetSizeIsClientSize() const override;
  bool IsModal() const override;
  int GetInitialShowState() const override;
  bool WillProcessWorkAreaChange() const override;
  int GetNonClientComponent(const gfx::Point& point) const override;
  ///void GetWindowMask(const gfx::Size& size, SkPath* path) override;
  bool GetClientAreaInsets(gfx::Insets* insets,
                           HMONITOR monitor) const override;
  bool GetDwmFrameInsetsInPixels(gfx::Insets* insets) const override;
  void GetMinMaxSize(gfx::Size* min_size, gfx::Size* max_size) const override;
  gfx::Size GetRootViewSize() const override;
  gfx::Size DIPToScreenSize(const gfx::Size& dip_size) const override;
  void ResetWindowControls() override;
  gfx::NativeViewAccessible GetNativeViewAccessible() override;
  void HandleActivationChanged(bool active) override;
  bool HandleAppCommand(short command) override;
  void HandleCancelMode() override;
  void HandleCaptureLost() override;
  void HandleClose() override;
  bool HandleCommand(int command) override;
  void HandleAccelerator(const crui::Accelerator& accelerator) override;
  void HandleCreate() override;
  void HandleDestroying() override;
  void HandleDestroyed() override;
  bool HandleInitialFocus(crui::WindowShowState show_state) override;
  void HandleDisplayChange() override;
  void HandleBeginWMSizeMove() override;
  void HandleEndWMSizeMove() override;
  void HandleMove() override;
  void HandleWorkAreaChanged() override;
  void HandleVisibilityChanging(bool visible) override;
  void HandleVisibilityChanged(bool visible) override;
  void HandleWindowMinimizedOrRestored(bool restored) override;
  void HandleClientSizeChanged(const gfx::Size& new_size) override;
  void HandleFrameChanged() override;
  void HandleNativeFocus(HWND last_focused_window) override;
  void HandleNativeBlur(HWND focused_window) override;
  bool HandleMouseEvent(crui::MouseEvent* event) override;
  void HandleKeyEvent(crui::KeyEvent* event) override;
  void HandleTouchEvent(crui::TouchEvent* event) override;
  bool HandleIMEMessage(UINT message,
                        WPARAM w_param,
                        LPARAM l_param,
                        LRESULT* result) override;
  void HandleInputLanguageChange(DWORD character_set,
                                 HKL input_language_id) override;
  void HandlePaintAccelerated(const gfx::Rect& invalid_rect) override;
  bool HandleTooltipNotify(int w_param,
                           NMHDR* l_param,
                           LRESULT* l_result) override;
  void HandleMenuLoop(bool in_menu_loop) override;
  bool PreHandleMSG(UINT message,
                    WPARAM w_param,
                    LPARAM l_param,
                    LRESULT* result) override;
  void PostHandleMSG(UINT message, WPARAM w_param, LPARAM l_param) override;
  bool HandleScrollEvent(crui::ScrollEvent* event) override;
  ///bool HandleGestureEvent(crui::GestureEvent* event) override;
  void HandleWindowSizeChanging() override;
  void HandleWindowSizeUnchanged() override;
  void HandleWindowScaleFactorChanged(float window_scale_factor) override;

 private:
  // internal::NativeWidgetPrivate:
  const Widget* GetWidgetImpl() const override;

  HWND GetHWND() const;
  gfx::Rect GetBoundsInPixels() const;
  void CheckForMonitorChange();
  void OnHostDisplayChanged();
  void OnFinalMessage();
  HMONITOR last_monitor_from_window_ = nullptr;

  std::unique_ptr<HWNDMessageHandler> message_handler_;

  // TODO(beng): Consider providing an interface to DesktopNativeWidgetAura
  //             instead of providing this route back to Widget.
  internal::NativeWidgetDelegate* native_widget_delegate_;

  Widget::InitParams::Ownership ownership_ 
      = Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET;

  // When certain windows are being shown, we augment the window size
  // temporarily for animation. The following two members contain the top left
  // and bottom right offsets which are used to enlarge the window.
  gfx::Vector2d window_expansion_top_left_delta_;
  gfx::Vector2d window_expansion_bottom_right_delta_;

  // Windows are enlarged to be at least 64x64 pixels, so keep track of the
  // extra added here.
  gfx::Vector2d window_enlargement_;

  typedef std::vector<std::unique_ptr<crui::ViewProp>> ViewProps;
  ViewProps props_;

  // True if the widget is going to have a non_client_view. We cache this value
  // rather than asking the Widget for the non_client_view so that we know at
  // Init time, before the Widget has created the NonClientView.
  bool has_non_client_view_;

  // True if the window should have the frame removed.
  bool remove_standard_frame_;

  // Indicates if current window will receive mouse events when should not
  // become activated.
  bool wants_mouse_events_when_inactive_ = false;

  // The z-order level of the window; the window exhibits "always on top"
  // behavior if > 0.
  crui::ZOrderLevel z_order_ = crui::ZOrderLevel::kNormal;
};


}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_NATIVE_WIDGET_WIN_H_
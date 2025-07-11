// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIN_HWND_MESSAGE_HANDLER_H_
#define UI_VIEWS_WIN_HWND_MESSAGE_HANDLER_H_

#include <windows.h>

#include <stddef.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "crbase/compiler_specific.h"
#include "crbase/memory/lazy_instance.h"
#include "crbase/memory/weak_ptr.h"
///#include "base/scoped_observer.h"
#include "crbase/strings/string16.h"
#include "crbase/win/win_util.h"
#include "crui/base/win/scoped_gdi_object.h"
///#include "crui/accessibility/ax_enums.mojom.h"
///#include "crui/accessibility/platform/ax_fragment_root_delegate_win.h"
///#include "crui/base/ime/input_method.h"
///#include "crui/base/ime/input_method_observer.h"
#include "crui/base/ui_base_types.h"
#include "crui/base/win/window_event_target.h"
#include "crui/base/ui_export.h"
#include "crui/events/event.h"
#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/sequential_id_generator.h"
#include "crui/gfx/win/msg_util.h"
#include "crui/gfx/win/window_impl.h"
#include "crui/views/win/pen_event_processor.h"
///#include "crui/views/win/scoped_enable_unadjusted_mouse_events_win.h"
#include "crui/views/window/window_resize_utils.h"

namespace crui {

namespace gfx {
///class ImageSkia;
class Insets;
class Canvas;
}  // namespace gfx

///class AXFragmentRootWin;
///class AXSystemCaretWin;
///class TextInputClient;
class ViewProp;
class SessionChangeObserver;

namespace views {

class FullscreenHandler;
class HWNDMessageHandlerDelegate;

// These two messages aren't defined in winuser.h, but they are sent to windows
// with captions. They appear to paint the window caption and frame.
// Unfortunately if you override the standard non-client rendering as we do
// with CustomFrameWindow, sometimes Windows (not deterministically
// reproducibly but definitely frequently) will send these messages to the
// window and paint the standard caption/title over the top of the custom one.
// So we need to handle these messages in CustomFrameWindow to prevent this
// from happening.
constexpr int WM_NCUAHDRAWCAPTION = 0xAE;
constexpr int WM_NCUAHDRAWFRAME = 0xAF;

// The HWNDMessageHandler sends this message to itself on
// WM_WINDOWPOSCHANGING. It's used to inform the client if a
// WM_WINDOWPOSCHANGED won't be received.
constexpr int WM_WINDOWSIZINGFINISHED = WM_USER;

// An object that handles messages for a HWND that implements the views
// "Custom Frame" look. The purpose of this class is to isolate the windows-
// specific message handling from the code that wraps it. It is intended to be
// used by both a views::NativeWidget and an aura::WindowTreeHost
// implementation.
// TODO(beng): This object should eventually *become* the WindowImpl.
class CRUI_EXPORT HWNDMessageHandler : public gfx::WindowImpl,
                                       ///public ui::InputMethodObserver,
                                       public crui::WindowEventTarget
                                       ///public ui::AXFragmentRootDelegateWin 
                                       {
 public:
  HWNDMessageHandler(const HWNDMessageHandler&) = delete;
  HWNDMessageHandler& operator=(const HWNDMessageHandler&) = delete;

  // See WindowImpl for details on |debugging_id|.
  HWNDMessageHandler(HWNDMessageHandlerDelegate* delegate,
                     const std::string& debugging_id);
  ~HWNDMessageHandler() override;

  void Init(HWND parent, const gfx::Rect& bounds);
  void InitModalType(crui::ModalType modal_type);

  void Close();
  void CloseNow();

  gfx::Rect GetWindowBoundsInScreen() const;
  gfx::Rect GetClientAreaBoundsInScreen() const;
  gfx::Rect GetRestoredBounds() const;
  // This accounts for the case where the widget size is the client size.
  gfx::Rect GetClientAreaBounds() const;

  void GetWindowPlacement(gfx::Rect* bounds,
                          crui::WindowShowState* show_state) const;

  // Sets the bounds of the HWND to |bounds_in_pixels|. If the HWND size is not
  // changed, |force_size_changed| determines if we should pretend it is.
  void SetBounds(const gfx::Rect& bounds_in_pixels, bool force_size_changed);

  void SetSize(const gfx::Size& size);
  void CenterWindow(const gfx::Size& size);

  void SetRegion(HRGN rgn);

  void StackAbove(HWND other_hwnd);
  void StackAtTop();

  // Shows the window. If |show_state| is maximized, |pixel_restore_bounds| is
  // the bounds to restore the window to when going back to normal.
  void Show(crui::WindowShowState show_state,
            const gfx::Rect& pixel_restore_bounds);
  void Hide();

  void Maximize();
  void Minimize();
  void Restore();

  void Activate();
  void Deactivate();

  void SetAlwaysOnTop(bool on_top);

  bool IsVisible() const;
  bool IsActive() const;
  bool IsMinimized() const;
  bool IsMaximized() const;
  bool IsFullscreen() const;
  bool IsAlwaysOnTop() const;

  bool RunMoveLoop(const gfx::Vector2d& drag_offset, bool hide_on_escape);
  void EndMoveLoop();

  // Tells the HWND its client area has changed.
  void SendFrameChanged();

  void FlashFrame(bool flash);

  void ClearNativeFocus();

  void SetCapture();
  void ReleaseCapture();
  bool HasCapture() const;

  FullscreenHandler* fullscreen_handler() { return fullscreen_handler_.get(); }

  void SetVisibilityChangedAnimationsEnabled(bool enabled);

  // Returns true if the title changed.
  bool SetTitle(const cr::string16& title);

  void SetCursor(HCURSOR cursor);

  void FrameTypeChanged();

  void SchedulePaintInRect(const gfx::Rect& rect);

  void SetOpacity(BYTE opcity);
  ///void SetWindowIcons(const gfx::ImageSkia& window_icon,
  ///                    const gfx::ImageSkia& app_icon);

  void set_use_system_default_icon(bool use_system_default_icon) {
    use_system_default_icon_ = use_system_default_icon;
  }

  void SetFullscreen(bool fullscreen);

  // Updates the aspect ratio of the window.
  void SetAspectRatio(float aspect_ratio);

  // Updates the window style to reflect whether it can be resized or maximized.
  void SizeConstraintsChanged();

  // Returns true if content is rendered to a child window instead of directly
  // to this window.
  bool HasChildRenderingWindow();

  void set_is_translucent(bool is_translucent) {
    is_translucent_ = is_translucent;
  }
  bool is_translucent() const { return is_translucent_; }

  ///std::unique_ptr<aura::ScopedEnableUnadjustedMouseEvents>
  ///RegisterUnadjustedMouseEvent();
  ///void set_using_wm_input(bool using_wm_input) {
  ///  using_wm_input_ = using_wm_input;
  ///}
  ///bool using_wm_input() { return using_wm_input_; }

 private:
  using TouchIDs = std::set<DWORD>;
  enum class DwmFrameState { kOff, kOn };

  // Overridden from WindowImpl:
  HICON GetDefaultWindowIcon() const override;
  HICON GetSmallWindowIcon() const override;
  LRESULT OnWndProc(UINT message, WPARAM w_param, LPARAM l_param) override;

  // Overridden from InputMethodObserver
  ///void OnFocus() override;
  ///void OnBlur() override;
  ///void OnCaretBoundsChanged(const ui::TextInputClient* client) override;
  ///void OnTextInputStateChanged(const ui::TextInputClient* client) override;
  ///void OnInputMethodDestroyed(const ui::InputMethod* input_method) override;
  ///void OnShowVirtualKeyboardIfEnabled() override;
  
  // Overridden from WindowEventTarget
  LRESULT HandleMouseMessage(unsigned int message,
                             WPARAM w_param,
                             LPARAM l_param,
                             bool* handled) override;
  LRESULT HandleKeyboardMessage(unsigned int message,
                                WPARAM w_param,
                                LPARAM l_param,
                                bool* handled) override;
  LRESULT HandleTouchMessage(unsigned int message,
                             WPARAM w_param,
                             LPARAM l_param,
                             bool* handled) override;
  LRESULT HandlePointerMessage(unsigned int message,
                               WPARAM w_param,
                               LPARAM l_param,
                               bool* handled) override;
  LRESULT HandleInputMessage(unsigned int message,
                             WPARAM w_param,
                             LPARAM l_param,
                             bool* handled) override;
  LRESULT HandleScrollMessage(unsigned int message,
                              WPARAM w_param,
                              LPARAM l_param,
                              bool* handled) override;
  LRESULT HandleNcHitTestMessage(unsigned int message,
                                 WPARAM w_param,
                                 LPARAM l_param,
                                 bool* handled) override;
  void HandleParentChanged() override;
  void ApplyPinchZoomScale(float scale) override;
  void ApplyPinchZoomBegin() override;
  void ApplyPinchZoomEnd() override;
  void ApplyPanGestureScroll(int scroll_x, int scroll_y) override;
  void ApplyPanGestureFling(int scroll_x, int scroll_y) override;
  void ApplyPanGestureScrollBegin(int scroll_x, int scroll_y) override;
  void ApplyPanGestureScrollEnd(bool transitioning_to_pinch) override;
  void ApplyPanGestureFlingBegin() override;
  void ApplyPanGestureFlingEnd() override;

  // Overridden from AXFragmentRootDelegateWin.
  ///gfx::NativeViewAccessible GetChildOfAXFragmentRoot() override;
  ///gfx::NativeViewAccessible GetParentOfAXFragmentRoot() override;
  ///bool IsAXFragmentRootAControlElement() override;

  void ApplyPanGestureEvent(int scroll_x,
                            int scroll_y,
                            crui::EventMomentumPhase momentum_phase,
                            crui::ScrollEventPhase phase);

  // Returns the auto-hide edges of the appbar. See
  // ViewsDelegate::GetAppbarAutohideEdges() for details. If the edges change,
  // OnAppbarAutohideEdgesChanged() is called.
  int GetAppbarAutohideEdges(HMONITOR monitor);

  // Callback if the autohide edges have changed. See
  // ViewsDelegate::GetAppbarAutohideEdges() for details.
  void OnAppbarAutohideEdgesChanged();

  // Can be called after the delegate has had the opportunity to set focus and
  // did not do so.
  void SetInitialFocus();

  // Called after the WM_ACTIVATE message has been processed by the default
  // windows procedure.
  void PostProcessActivateMessage(
      int activation_state,
      bool minimized,
      HWND window_gaining_or_losing_activation);

  // Enables disabled owner windows that may have been disabled due to this
  // window's modality.
  void RestoreEnabledIfNecessary();

  // Executes the specified SC_command.
  void ExecuteSystemMenuCommand(int command);

  // Start tracking all mouse events so that this window gets sent mouse leave
  // messages too.
  void TrackMouseEvents(DWORD mouse_tracking_flags);

  // Responds to the client area changing size, either at window creation time
  // or subsequently.
  void ClientAreaSizeChanged();

  // Returns true if |insets| was modified to define a custom client area for
  // the window, false if the default client area should be used. If false is
  // returned, |insets| is not modified.  |monitor| is the monitor this
  // window is on.  Normally that would be determined from the HWND, but
  // during WM_NCCALCSIZE Windows does not return the correct monitor for the
  // HWND, so it must be passed in explicitly (see HWNDMessageHandler::
  // OnNCCalcSize for more details).
  bool GetClientAreaInsets(gfx::Insets* insets, HMONITOR monitor) const;

  // Resets the window region for the current widget bounds if necessary.
  // If |force| is true, the window region is reset to NULL even for native
  // frame windows.
  void ResetWindowRegion(bool force, bool redraw);

  // Enables or disables rendering of the non-client (glass) area by DWM,
  // under Vista and above, depending on whether the caller has requested a
  // custom frame.
  void UpdateDwmNcRenderingPolicy();

  // Calls DefWindowProc, safely wrapping the call in a ScopedRedrawLock to
  // prevent frame flicker. DefWindowProc handling can otherwise render the
  // classic-look window title bar directly.
  LRESULT DefWindowProcWithRedrawLock(UINT message,
                                      WPARAM w_param,
                                      LPARAM l_param);

  // Lock or unlock the window from being able to redraw itself in response to
  // updates to its invalid region.
  class ScopedRedrawLock;
  void LockUpdates();
  void UnlockUpdates();

  // Stops ignoring SetWindowPos() requests (see below).
  void StopIgnoringPosChanges() { ignore_window_pos_changes_ = false; }

  // Synchronously updates the invalid contents of the Widget. Valid for
  // layered windows only.
  void RedrawLayeredWindowContents();

  // Attempts to force the window to be redrawn, ensuring that it gets
  // onscreen.
  void ForceRedrawWindow(int attempts);

  // Returns whether Windows should help with frame rendering (i.e. we're using
  // the glass frame).
  bool IsFrameSystemDrawn() const;

  // Returns true if IsFrameSystemDrawn() and there's actually a frame to draw.
  bool HasSystemFrame() const;

  // Adds or removes the frame extension into client area with
  // DwmExtendFrameIntoClientArea.
  void SetDwmFrameExtension(DwmFrameState state);

  // Message Handlers ----------------------------------------------------------

  CR_BEGIN_MSG_MAP_EX(HWNDMessageHandler)
    // Range handlers must go first!
    CR_MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseRange)
    CR_MESSAGE_RANGE_HANDLER_EX(WM_NCMOUSEMOVE, WM_NCXBUTTONDBLCLK,
                                OnMouseRange)

    // CustomFrameWindow hacks
    CR_MESSAGE_HANDLER_EX(WM_NCUAHDRAWCAPTION, OnNCUAHDrawCaption)
    CR_MESSAGE_HANDLER_EX(WM_NCUAHDRAWFRAME, OnNCUAHDrawFrame)

    // Vista and newer
    CR_MESSAGE_HANDLER_EX(WM_DWMCOMPOSITIONCHANGED, OnDwmCompositionChanged)

    // Win 8.1 and newer
    CR_MESSAGE_HANDLER_EX(WM_DPICHANGED, OnDpiChanged)

    // Non-atlcrack.h handlers
    CR_MESSAGE_HANDLER_EX(WM_GETOBJECT, OnGetObject)

    // Mouse events.
    CR_MESSAGE_HANDLER_EX(WM_MOUSEACTIVATE, OnMouseActivate)
    CR_MESSAGE_HANDLER_EX(WM_MOUSELEAVE, OnMouseRange)
    CR_MESSAGE_HANDLER_EX(WM_NCMOUSELEAVE, OnMouseRange)
    CR_MESSAGE_HANDLER_EX(WM_SETCURSOR, OnSetCursor);

    // Pointer events.
    CR_MESSAGE_HANDLER_EX(WM_POINTERACTIVATE, OnPointerActivate)
    CR_MESSAGE_HANDLER_EX(WM_POINTERDOWN, OnPointerEvent)
    CR_MESSAGE_HANDLER_EX(WM_POINTERUP, OnPointerEvent)
    CR_MESSAGE_HANDLER_EX(WM_POINTERUPDATE, OnPointerEvent)
    CR_MESSAGE_HANDLER_EX(WM_POINTERENTER, OnPointerEvent)
    CR_MESSAGE_HANDLER_EX(WM_POINTERLEAVE, OnPointerEvent)
    CR_MESSAGE_HANDLER_EX(WM_NCPOINTERDOWN, OnPointerEvent)
    CR_MESSAGE_HANDLER_EX(WM_NCPOINTERUP, OnPointerEvent)
    CR_MESSAGE_HANDLER_EX(WM_NCPOINTERUPDATE, OnPointerEvent)

    // Key events.
    CR_MESSAGE_HANDLER_EX(WM_KEYDOWN, OnKeyEvent)
    CR_MESSAGE_HANDLER_EX(WM_KEYUP, OnKeyEvent)
    CR_MESSAGE_HANDLER_EX(WM_SYSKEYDOWN, OnKeyEvent)
    CR_MESSAGE_HANDLER_EX(WM_SYSKEYUP, OnKeyEvent)

    CR_MESSAGE_HANDLER_EX(WM_INPUT, OnInputEvent)
    // IME Events.
    CR_MESSAGE_HANDLER_EX(WM_IME_SETCONTEXT, OnImeMessages)
    CR_MESSAGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, OnImeMessages)
    CR_MESSAGE_HANDLER_EX(WM_IME_COMPOSITION, OnImeMessages)
    CR_MESSAGE_HANDLER_EX(WM_IME_ENDCOMPOSITION, OnImeMessages)
    CR_MESSAGE_HANDLER_EX(WM_IME_REQUEST, OnImeMessages)
    CR_MESSAGE_HANDLER_EX(WM_IME_NOTIFY, OnImeMessages)
    CR_MESSAGE_HANDLER_EX(WM_CHAR, OnImeMessages)
    CR_MESSAGE_HANDLER_EX(WM_SYSCHAR, OnImeMessages)

    // Scroll events
    CR_MESSAGE_HANDLER_EX(WM_VSCROLL, OnScrollMessage)
    CR_MESSAGE_HANDLER_EX(WM_HSCROLL, OnScrollMessage)

    // Touch Events.
    CR_MESSAGE_HANDLER_EX(WM_TOUCH, OnTouchEvent)

    CR_MESSAGE_HANDLER_EX(WM_WINDOWSIZINGFINISHED, OnWindowSizingFinished)

    // Uses the general handler macro since the specific handler macro
    // MSG_WM_NCACTIVATE would convert WPARAM type to BOOL type. The high
    // word of WPARAM could be set when the window is minimized or restored.
    CR_MESSAGE_HANDLER_EX(WM_NCACTIVATE, OnNCActivate)

    // This list is in _ALPHABETICAL_ order! OR I WILL HURT YOU.
    CR_MSG_WM_ACTIVATEAPP(OnActivateApp)
    CR_MSG_WM_APPCOMMAND(OnAppCommand)
    CR_MSG_WM_CANCELMODE(OnCancelMode)
    CR_MSG_WM_CAPTURECHANGED(OnCaptureChanged)
    CR_MSG_WM_CLOSE(OnClose)
    CR_MSG_WM_COMMAND(OnCommand)
    CR_MSG_WM_CREATE(OnCreate)
    CR_MSG_WM_DESTROY(OnDestroy)
    CR_MSG_WM_DISPLAYCHANGE(OnDisplayChange)
    CR_MSG_WM_ENTERMENULOOP(OnEnterMenuLoop)
    CR_MSG_WM_EXITMENULOOP(OnExitMenuLoop)
    CR_MSG_WM_ENTERSIZEMOVE(OnEnterSizeMove)
    CR_MSG_WM_ERASEBKGND(OnEraseBkgnd)
    CR_MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
    CR_MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
    CR_MSG_WM_INITMENU(OnInitMenu)
    CR_MSG_WM_INPUTLANGCHANGE(OnInputLangChange)
    CR_MSG_WM_KILLFOCUS(OnKillFocus)
    CR_MSG_WM_MOVE(OnMove)
    CR_MSG_WM_MOVING(OnMoving)
    CR_MSG_WM_NCCALCSIZE(OnNCCalcSize)
    CR_MSG_WM_NCCREATE(OnNCCreate)
    CR_MSG_WM_NCHITTEST(OnNCHitTest)
    CR_MSG_WM_NCPAINT(OnNCPaint)
    CR_MSG_WM_NOTIFY(OnNotify)
    CR_MSG_WM_PAINT(OnPaint)
    CR_MSG_WM_SETFOCUS(OnSetFocus)
    CR_MSG_WM_SETICON(OnSetIcon)
    CR_MSG_WM_SETTEXT(OnSetText)
    CR_MSG_WM_SETTINGCHANGE(OnSettingChange)
    CR_MSG_WM_SIZE(OnSize)
    CR_MSG_WM_SIZING(OnSizing)
    CR_MSG_WM_SYSCOMMAND(OnSysCommand)
    CR_MSG_WM_THEMECHANGED(OnThemeChanged)
    CR_MSG_WM_TIMECHANGE(OnTimeChange)
    CR_MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
    CR_MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)
  CR_END_MSG_MAP()

  // Message Handlers.
  // This list is in _ALPHABETICAL_ order!
  // TODO(beng): Once this object becomes the WindowImpl, these methods can
  //             be made private.
  void OnActivateApp(BOOL active, DWORD thread_id);
  // TODO(beng): return BOOL is temporary until this object becomes a
  //             WindowImpl.
  BOOL OnAppCommand(HWND window, short command, WORD device, int keystate);
  void OnCancelMode();
  void OnCaptureChanged(HWND window);
  void OnClose();
  void OnCommand(UINT notification_code, int command, HWND window);
  LRESULT OnCreate(CREATESTRUCT* create_struct);
  void OnDestroy();
  void OnDisplayChange(UINT bits_per_pixel, const gfx::Size& screen_size);
  LRESULT OnDpiChanged(UINT msg, WPARAM w_param, LPARAM l_param);
  LRESULT OnDwmCompositionChanged(UINT msg, WPARAM w_param, LPARAM l_param);
  void OnEnterMenuLoop(BOOL from_track_popup_menu);
  void OnEnterSizeMove();
  LRESULT OnEraseBkgnd(HDC dc);
  void OnExitMenuLoop(BOOL is_shortcut_menu);
  void OnExitSizeMove();
  void OnGetMinMaxInfo(MINMAXINFO* minmax_info);
  LRESULT OnGetObject(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnImeMessages(UINT message, WPARAM w_param, LPARAM l_param);
  void OnInitMenu(HMENU menu);
  LRESULT OnInputEvent(UINT message, WPARAM w_param, LPARAM l_param);
  void OnInputLangChange(DWORD character_set, HKL input_language_id);
  LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param);
  void OnKillFocus(HWND focused_window);
  LRESULT OnMouseActivate(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnMouseRange(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnPointerActivate(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnPointerEvent(UINT message, WPARAM w_param, LPARAM l_param);
  void OnMove(const gfx::Point& point);
  void OnMoving(UINT param, const RECT* new_bounds);
  LRESULT OnNCActivate(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnNCCalcSize(BOOL mode, LPARAM l_param);
  LRESULT OnNCCreate(LPCREATESTRUCT lpCreateStruct);
  LRESULT OnNCHitTest(const gfx::Point& point);
  void OnNCPaint(HRGN rgn);
  LRESULT OnNCUAHDrawCaption(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnNCUAHDrawFrame(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnNotify(int w_param, NMHDR* l_param);
  void OnPaint(HDC dc);
  LRESULT OnReflectedMessage(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnScrollMessage(UINT message, WPARAM w_param, LPARAM l_param);
  LRESULT OnSetCursor(UINT message, WPARAM w_param, LPARAM l_param);
  void OnSetFocus(HWND last_focused_window);
  LRESULT OnSetIcon(UINT size_type, HICON new_icon);
  LRESULT OnSetText(const wchar_t* text);
  void OnSettingChange(UINT flags, const wchar_t* section);
  void OnSize(UINT param, const gfx::Size& size);
  void OnSizing(UINT param, RECT* rect);
  void OnSysCommand(UINT notification_code, const gfx::Point& point);
  void OnThemeChanged();
  void OnTimeChange();
  LRESULT OnTouchEvent(UINT message, WPARAM w_param, LPARAM l_param);
  void OnWindowPosChanging(WINDOWPOS* window_pos);
  void OnWindowPosChanged(WINDOWPOS* window_pos);
  LRESULT OnWindowSizingFinished(UINT message, WPARAM w_param, LPARAM l_param);

  // Receives Windows Session Change notifications.
  void OnSessionChange(WPARAM status_code, const bool* is_current_session);

  using TouchEvents = std::vector<crui::TouchEvent>;
  // Helper to handle the list of touch events passed in. We need this because
  // touch events on windows don't fire if we enter a modal loop in the context
  // of a touch event.
  void HandleTouchEvents(const TouchEvents& touch_events);

  // Resets the flag which indicates that we are in the context of a touch down
  // event.
  void ResetTouchDownContext();

  // Helper to handle mouse events.
  // The |message|, |w_param|, |l_param| parameters identify the Windows mouse
  // message and its parameters respectively.
  // The |track_mouse| parameter indicates if we should track the mouse.
  LRESULT HandleMouseEventInternal(UINT message,
                                   WPARAM w_param,
                                   LPARAM l_param,
                                   bool track_mouse);

  LRESULT HandlePointerEventTypeTouch(UINT message,
                                      WPARAM w_param,
                                      LPARAM l_param);

  LRESULT HandlePointerEventTypePen(UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param);

  // Returns true if the mouse message passed in is an OS synthesized mouse
  // message.
  // |message| identifies the mouse message.
  // |message_time| is the time when the message occurred.
  // |l_param| indicates the location of the mouse message.
  bool IsSynthesizedMouseMessage(unsigned int message,
                                 int message_time,
                                 LPARAM l_param);

  // Provides functionality to transition a frame to DWM.
  void PerformDwmTransition();

  // Updates DWM frame to extend into client area if needed.
  void UpdateDwmFrame();

  // Generates a touch event and adds it to the |touch_events| parameter.
  // |point| is the point where the touch was initiated.
  // |id| is the event id associated with the touch event.
  // |time_stamp| is the time stamp associated with the message.
  void GenerateTouchEvent(crui::EventType event_type,
                          const gfx::Point& point,
                          size_t id,
                          cr::TimeTicks time_stamp,
                          TouchEvents* touch_events);

  // Handles WM_NCLBUTTONDOWN and WM_NCMOUSEMOVE messages on the caption.
  // Returns true if the message was handled.
  bool HandleMouseInputForCaption(unsigned int message,
                                  WPARAM w_param,
                                  LPARAM l_param);

  // Helper function for setting the bounds of the HWND. For more information
  // please refer to the SetBounds() function.
  void SetBoundsInternal(const gfx::Rect& bounds_in_pixels,
                         bool force_size_changed);

  // Checks if there is a full screen window on the same monitor as the
  // |window| which is becoming active. If yes then we reduce the size of the
  // fullscreen window by 1 px to ensure that maximized windows on the same
  // monitor don't draw over the taskbar.
  void CheckAndHandleBackgroundFullscreenOnMonitor(HWND window);

  // Provides functionality to reduce the bounds of the fullscreen window by 1
  // px on activation loss to a window on the same monitor.
  void OnBackgroundFullscreen();

  // Deletes the system caret used for accessibility. This will result in any
  // clients that are still holding onto its |IAccessible| to get a failure code
  // if they request its location.
  void DestroyAXSystemCaret();

  // Updates |rect| to adhere to the |aspect_ratio| of the window. |param|
  // refers to the edge of the window being sized.
  void SizeRectToAspectRatio(UINT param, gfx::Rect* rect);

  // Get the cursor position, which may be mocked if running a test
  POINT GetCursorPos() const;

  HWNDMessageHandlerDelegate* delegate_;

  std::unique_ptr<FullscreenHandler> fullscreen_handler_;

  // Set to true in Close() and false is CloseNow().
  bool waiting_for_close_now_;

  bool use_system_default_icon_;

  // Whether all ancestors have been enabled. This is only used if is_modal_ is
  // true.
  bool restored_enabled_;

  // The current cursor.
  HCURSOR current_cursor_;

  // The last cursor that was active before the current one was selected. Saved
  // so that we can restore it.
  HCURSOR previous_cursor_;

  // The icon created from the bitmap image of the window icon.
  crui::win::ScopedHICON window_icon_;

  // The icon created from the bitmap image of the app icon.
  crui::win::ScopedHICON app_icon_;

  // The aspect ratio for the window. This is only used for sizing operations
  // for the non-client area.
  cr::Optional<float> aspect_ratio_;

  // The current DPI.
  int dpi_;

  // Whether EnableNonClientDpiScaling was called successfully with this window.
  // This flag exists because EnableNonClientDpiScaling must be called during
  // WM_NCCREATE and EnableChildWindowDpiMessage is called after window
  // creation. We don't want to call both, so this helps us determine if a call
  // to EnableChildWindowDpiMessage is necessary.
  bool called_enable_non_client_dpi_scaling_;

  // Event handling ------------------------------------------------------------

  // The flags currently being used with TrackMouseEvent to track mouse
  // messages. 0 if there is no active tracking. The value of this member is
  // used when tracking is canceled.
  DWORD active_mouse_tracking_flags_;

  // Set to true when the user presses the right mouse button on the caption
  // area. We need this so we can correctly show the context menu on mouse-up.
  bool is_right_mouse_pressed_on_caption_;

  // The set of touch devices currently down.
  TouchIDs touch_ids_;

  // ScopedRedrawLock ----------------------------------------------------------

  // Represents the number of ScopedRedrawLocks active against this widget.
  // If this is greater than zero, the widget should be locked against updates.
  int lock_updates_count_;

  // Window resizing -----------------------------------------------------------

  // When true, this flag makes us discard incoming SetWindowPos() requests that
  // only change our position/size.  (We still allow changes to Z-order,
  // activation, etc.)
  bool ignore_window_pos_changes_;

  // Keeps track of the last size type param received from a WM_SIZE message.
  UINT last_size_param_ = SIZE_RESTORED;

  // The last-seen monitor containing us, and its rect and work area.  These are
  // used to catch updates to the rect and work area and react accordingly.
  HMONITOR last_monitor_;
  gfx::Rect last_monitor_rect_, last_work_area_;

  // True the first time nccalc is called on a sizable widget
  bool is_first_nccalc_;
  
  // Layered windows -----------------------------------------------------------

  // Should we keep an off-screen buffer? This is false by default, set to true
  // when WS_EX_LAYERED is specified before the native window is created.
  //
  // NOTE: this is intended to be used with a layered window (a window with an
  // extended window style of WS_EX_LAYERED). If you are using a layered window
  // and NOT changing the layered alpha or anything else, then leave this value
  // alone. OTOH if you are invoking SetLayeredWindowAttributes then you'll
  // most likely want to set this to false, or after changing the alpha toggle
  // the extended style bit to false than back to true. See MSDN for more
  // details.
  bool use_layered_buffer_;

  // The default alpha to be applied to the layered window.
  BYTE layered_alpha_;

  // A canvas that contains the window contents in the case of a layered
  // window.
  std::unique_ptr<gfx::Canvas> layered_window_contents_;

  // We must track the invalid rect ourselves, for two reasons:
  // For layered windows, Windows will not do this properly with
  // InvalidateRect()/GetUpdateRect(). (In fact, it'll return misleading
  // information from GetUpdateRect()).
  // We also need to keep track of the invalid rectangle for the RootView should
  // we need to paint the non-client area. The data supplied to WM_NCPAINT seems
  // to be insufficient.
  gfx::Rect invalid_rect_;

  // Set to true when waiting for RedrawLayeredWindowContents().
  bool waiting_for_redraw_layered_window_contents_;

  // Copy of custom window region specified via SetRegion(), if any.
  crui::win::ScopedRegion custom_window_region_;

  // If > 0 indicates a menu is running (we're showing a native menu).
  int menu_depth_;

  // Generates touch-ids for touch-events.
  crui::SequentialIDGenerator id_generator_;

  PenEventProcessor pen_processor_;

  // Stores a pointer to the WindowEventTarget interface implemented by this
  // class. Allows callers to retrieve the interface pointer.
  std::unique_ptr<crui::ViewProp> prop_window_target_;

  // Number of active touch down contexts. This is incremented on touch down
  // events and decremented later using a delayed task.
  // We need this to ignore WM_MOUSEACTIVATE messages generated in response to
  // touch input. This is fine because activation still works correctly via
  // native SetFocus calls invoked in the views code.
  int touch_down_contexts_;

  // Time the last touch or pen message was received. Used to flag mouse
  // messages synthesized by Windows for touch which are not flagged by the OS
  // as synthesized mouse messages. For more information please refer to the
  // IsMouseEventFromTouch function.
  static long last_touch_or_pen_message_time_;

  // Time the last WM_MOUSEHWHEEL message is received. Please refer to the
  // HandleMouseEventInternal function as to why this is needed.
  long last_mouse_hwheel_time_;

  // On Windows Vista and beyond, if we are transitioning from custom frame
  // to Aero(glass) we delay setting the DWM related properties in full
  // screen mode as DWM is not supported in full screen windows. We perform
  // the DWM related operations when the window comes out of fullscreen mode.
  // This member variable is set to true if the window is transitioning to
  // glass. Defaults to false.
  bool dwm_transition_desired_;

  // Is DWM composition currently enabled?
  // Note: According to MSDN docs for DwmIsCompositionEnabled(), this is always
  // true starting in Windows 8.
  bool dwm_composition_enabled_;

  // True if HandleWindowSizeChanging has been called in the delegate, but not
  // HandleClientSizeChanged.
  bool sent_window_size_changing_;

  // This is used to keep track of whether a WM_WINDOWPOSCHANGED has
  // been received after the WM_WINDOWPOSCHANGING.
  uint32_t current_window_size_message_ = 0;

  // Manages observation of Windows Session Change messages.
  std::unique_ptr<crui::SessionChangeObserver> session_change_observer_;

  // Some assistive software need to track the location of the caret.
  ///std::unique_ptr<ui::AXSystemCaretWin> ax_system_caret_;

  // Implements IRawElementProviderFragmentRoot when UIA is enabled
  ///std::unique_ptr<ui::AXFragmentRootWin> ax_fragment_root_;

  // The location where the user clicked on the caption. We cache this when we
  // receive the WM_NCLBUTTONDOWN message. We use this in the subsequent
  // WM_NCMOUSEMOVE message to see if the mouse actually moved.
  // Please refer to the HandleMouseEventInternal function for details on why
  // this is needed.
  gfx::Point caption_left_button_click_pos_;

  // Set to true if the left mouse button has been pressed on the caption.
  // Defaults to false.
  bool left_button_down_on_caption_;

  // Set to true if the window is a background fullscreen window, i.e a
  // fullscreen window which lost activation. Defaults to false.
  bool background_fullscreen_hack_;

  // True if the window should have no border and its contents should be
  // partially or fully transparent.
  bool is_translucent_ = false;

  // True if the window should process WM_POINTER for touch events and
  // not WM_TOUCH events.
  bool pointer_events_for_touch_;

  // True if DWM frame should be cleared on next WM_ERASEBKGND message.  This is
  // necessary to avoid white flashing in the titlebar area around the
  // minimize/maximize/close buttons.  Clearing the frame on every WM_ERASEBKGND
  // message causes black flickering in the titlebar region so we do it on for
  // the first message after frame type changes.
  bool needs_dwm_frame_clear_ = true;

  // True if is handling mouse WM_INPUT messages.
  bool using_wm_input_ = false;

  // This is a map of the HMONITOR to full screeen window instance. It is safe
  // to keep a raw pointer to the HWNDMessageHandler instance as we track the
  // window destruction and ensure that the map is cleaned up.
  using FullscreenWindowMonitorMap = std::map<HMONITOR, HWNDMessageHandler*>;
  static cr::LazyInstance<FullscreenWindowMonitorMap>::DestructorAtExit
      fullscreen_monitor_map_;

  // Populated if the cursor position is being mocked for testing purposes.
  cr::Optional<gfx::Point> mock_cursor_position_;

  ///ScopedObserver<ui::InputMethod, ui::InputMethodObserver> observer_{this};

  // The WeakPtrFactories below (one inside the
  // CR_MSG_MAP_CLASS_DECLARATIONS macro and autohide_factory_) must
  // occur last in the class definition so they get destroyed last.

  CR_MSG_MAP_CLASS_DECLARATIONS(HWNDMessageHandler)

  // The factory used to lookup appbar autohide edges.
  cr::WeakPtrFactory<HWNDMessageHandler> autohide_factory_{this};
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIN_HWND_MESSAGE_HANDLER_H_

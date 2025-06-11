// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_BUTTON_BUTTON_H_
#define UI_VIEWS_CONTROLS_BUTTON_BUTTON_H_

#include <memory>

#include "crbase/functional/bind.h"
#include "crui/events/event_constants.h"
#include "crui/gfx/animation/throb_animation.h"
///#include "ui/native_theme/native_theme.h"
#include "crui/views/animation/animation_delegate_views.h"
///#include "ui/views/animation/ink_drop_host_view.h"
///#include "ui/views/animation/ink_drop_state.h"
#include "crui/events/event.h"
#include "crui/views/controls/button/button_controller_delegate.h"
#include "crui/views/controls/focus_ring.h"
#include "crui/views/painter.h"
#include "crui/views/widget/widget_observer.h"

namespace crui {
namespace views {

class Button;
class ButtonController;
class ButtonObserver;
class Event;

// An interface implemented by an object to let it know that a button was
// pressed.
class CRUI_EXPORT ButtonListener {
 public:
  virtual void ButtonPressed(Button* sender, const crui::Event& event) = 0;

 protected:
  virtual ~ButtonListener() = default;
};

// A View representing a button. A Button is not focusable by default and will
// not be part of the focus chain, unless in accessibility mode (see
// SetFocusForPlatform()).
class CRUI_EXPORT Button : public View,///InkDropHostView,
                           public AnimationDelegateViews {
 public:
  METADATA_HEADER(Button)

  Button(const Button&) = delete;
  Button& operator=(const Button&) = delete;

  // Construct the Button with a Listener. The listener can be null. This can be
  // true of buttons that don't have a listener - e.g. menubuttons where there's
  // no default action and checkboxes.
  explicit Button(ButtonListener* listener);
  ~Button() override;

  // Button states for various button sub-types.
  enum ButtonState {
    STATE_NORMAL = 0,
    STATE_HOVERED,
    STATE_PRESSED,
    STATE_DISABLED,
    STATE_COUNT,
  };

  // An enum describing the events on which a button should be clicked for a
  // given key event.
  enum class KeyClickAction {
    kOnKeyPress,
    kOnKeyRelease,
    kNone,
  };

  // TODO(cyan): Consider having Button implement ButtonControllerDelegate.
  class CRUI_EXPORT DefaultButtonControllerDelegate
      : public ButtonControllerDelegate {
   public:
    DefaultButtonControllerDelegate(
        const DefaultButtonControllerDelegate&) = delete;
    DefaultButtonControllerDelegate& operator=(
        const DefaultButtonControllerDelegate&) = delete;

    explicit DefaultButtonControllerDelegate(Button* button);
    ~DefaultButtonControllerDelegate() override;

    // views::ButtonControllerDelegate:
    void RequestFocusFromEvent() override;
    void NotifyClick(const crui::Event& event) override;
    void OnClickCanceled(const crui::Event& event) override;
    bool IsTriggerableEvent(const crui::Event& event) override;
    bool ShouldEnterPushedState(const crui::Event& event) override;
    bool ShouldEnterHoveredState() override;
    ///InkDrop* GetInkDrop() override;
    ///int GetDragOperations(const gfx::Point& press_pt) override;
    bool InDrag() override;
  };

  static const Button* AsButton(const View* view);
  static Button* AsButton(View* view);

  ///static ButtonState GetButtonStateFrom(crui::NativeTheme::State state);

  // Make the button focusable as per the platform.
  void SetFocusForPlatform();

  void SetTooltipText(const cr::string16& tooltip_text);

  int tag() const { return tag_; }
  void set_tag(int tag) { tag_ = tag; }

  void SetAccessibleName(const cr::string16& name);
  const cr::string16& GetAccessibleName() const;

  // Get/sets the current display state of the button.
  ButtonState state() const { return state_; }
  // Clients passing in STATE_DISABLED should consider calling
  // SetEnabled(false) instead because the enabled flag can affect other things
  // like event dispatching, focus traversals, etc. Calling SetEnabled(false)
  // will also set the state of |this| to STATE_DISABLED.
  void SetState(ButtonState state);
  // Returns the visual appearance state of the button. This takes into account
  // both the button's display state and the state of the containing widget.
  ButtonState GetVisualState() const;

  // Starts throbbing. See HoverAnimation for a description of cycles_til_stop.
  // This method does nothing if |animate_on_state_change_| is false.
  void StartThrobbing(int cycles_til_stop);

  // Stops throbbing immediately.
  void StopThrobbing();

  // Set how long the hover animation will last for.
  void SetAnimationDuration(cr::TimeDelta duration);

  void set_triggerable_event_flags(int triggerable_event_flags) {
    triggerable_event_flags_ = triggerable_event_flags;
  }
  int triggerable_event_flags() const { return triggerable_event_flags_; }

  // Sets whether |RequestFocus| should be invoked on a mouse press. The default
  // is false.
  void set_request_focus_on_press(bool value) {
// On Mac, buttons should not request focus on a mouse press. Hence keep the
// default value i.e. false.
#if !defined(MINI_CHROMIUM_OS_MACOSX)
    request_focus_on_press_ = value;
#endif
  }

  bool request_focus_on_press() const { return request_focus_on_press_; }

  // See description above field.
  void set_animate_on_state_change(bool value) {
    animate_on_state_change_ = value;
  }

  void SetInstallFocusRingOnFocus(bool install_focus_ring_on_focus);

  void SetHotTracked(bool is_hot_tracked);
  bool IsHotTracked() const;

  void SetFocusPainter(std::unique_ptr<Painter> focus_painter);

  // Highlights the ink drop for the button.
  void SetHighlighted(bool bubble_visible);

  void AddButtonObserver(ButtonObserver* observer);
  void RemoveButtonObserver(ButtonObserver* observer);

  // Overridden from View:
  bool OnMousePressed(const crui::MouseEvent& event) override;
  bool OnMouseDragged(const crui::MouseEvent& event) override;
  void OnMouseReleased(const crui::MouseEvent& event) override;
  void OnMouseCaptureLost() override;
  void OnMouseEntered(const crui::MouseEvent& event) override;
  void OnMouseExited(const crui::MouseEvent& event) override;
  void OnMouseMoved(const crui::MouseEvent& event) override;
  bool OnKeyPressed(const crui::KeyEvent& event) override;
  bool OnKeyReleased(const crui::KeyEvent& event) override;
  void OnGestureEvent(crui::GestureEvent* event) override;
  bool AcceleratorPressed(const crui::Accelerator& accelerator) override;
  bool SkipDefaultKeyEventProcessing(const crui::KeyEvent& event) override;
  cr::string16 GetTooltipText(const gfx::Point& p) const override;
  void ShowContextMenu(const gfx::Point& p,
                       crui::MenuSourceType source_type) override;
  ///void OnDragDone() override;
  // Instead of overriding this, subclasses that want custom painting should use
  // PaintButtonContents.
  void OnPaint(gfx::Canvas* canvas) final;
  ///void GetAccessibleNodeData(ui::AXNodeData* node_data) override;
  void VisibilityChanged(View* starting_from, bool is_visible) override;
  void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) override;
  void OnFocus() override;
  void OnBlur() override;
  void AddedToWidget() override;
  void RemovedFromWidget() override;

  // Overridden from InkDropHostView:
  ///std::unique_ptr<InkDrop> CreateInkDrop() override;
  ///SkColor GetInkDropBaseColor() const override;

  // Overridden from views::AnimationDelegateViews:
  void AnimationProgressed(const gfx::Animation* animation) override;

  // Returns the click action for the given key event.
  // Subclasses may override this method to support default actions for key
  // events.
  // TODO(cyan): Move this into the ButtonController.
  virtual KeyClickAction GetKeyClickActionForEvent(const crui::KeyEvent& event);

  ButtonController* button_controller() const {
    return button_controller_.get();
  }

  void SetButtonController(std::unique_ptr<ButtonController> button_controller);

  gfx::Point GetMenuPosition() const;

 protected:

  // Called when the button has been clicked or tapped and should request focus
  // if necessary.
  virtual void RequestFocusFromEvent();

  // Cause the button to notify the listener that a click occurred.
  virtual void NotifyClick(const crui::Event& event);

  // Called when a button gets released without triggering an action.
  // Note: This is only wired up for mouse button events and not gesture
  // events.
  virtual void OnClickCanceled(const crui::Event& event);

  // Called when the tooltip is set.
  virtual void OnSetTooltipText(const cr::string16& tooltip_text);

  // Invoked from SetState() when SetState() is passed a value that differs from
  // the current node_data. Button's implementation of StateChanged() does
  // nothing; this method is provided for subclasses that wish to do something
  // on state changes.
  virtual void StateChanged(ButtonState old_state);

  // Returns true if the event is one that can trigger notifying the listener.
  // This implementation returns true if the left mouse button is down.
  // TODO(cyan): Remove this method and move the implementation into
  // ButtonController.
  virtual bool IsTriggerableEvent(const crui::Event& event);

  // Returns true if the ink drop should be updated by Button when
  // OnClickCanceled() is called. This method is provided for subclasses.
  // If the method is overriden and returns false, the subclass is responsible
  // will be responsible for updating the ink drop.
  virtual bool ShouldUpdateInkDropOnClickCanceled() const;

  // Returns true if the button should become pressed when the user
  // holds the mouse down over the button. For this implementation,
  // we simply return IsTriggerableEvent(event).
  virtual bool ShouldEnterPushedState(const crui::Event& event);

  // Override to paint custom button contents. Any background or border set on
  // the view will be painted before this is called and |focus_painter_| will be
  // painted afterwards.
  virtual void PaintButtonContents(gfx::Canvas* canvas);

  // Returns true if the button should enter hovered state; that is, if the
  // mouse is over the button, and no other window has capture (which would
  // prevent the button from receiving MouseExited events and updating its
  // node_data). This does not take into account enabled node_data.
  bool ShouldEnterHoveredState();

  const gfx::ThrobAnimation& hover_animation() const {
    return hover_animation_;
  }

  FocusRing* focus_ring() { return focus_ring_.get(); }

  // The button's listener. Notified when clicked.
  ButtonListener* listener_;

 private:
  // Bridge class to allow Button to observe a Widget without being a
  // WidgetObserver. This is desirable because many Button subclasses are
  // themselves WidgetObservers, and if Button is a WidgetObserver, any change
  // to its WidgetObserver overrides requires updating all the subclasses as
  // well.
  class WidgetObserverButtonBridge : public WidgetObserver {
   public:
    WidgetObserverButtonBridge(
        const WidgetObserverButtonBridge&) = delete;
    WidgetObserverButtonBridge& operator=(
        const WidgetObserverButtonBridge&) = delete;

    explicit WidgetObserverButtonBridge(Button* owner);
    ~WidgetObserverButtonBridge() override;

    // WidgetObserver:
    void OnWidgetActivationChanged(Widget* widget, bool active) override;
    void OnWidgetDestroying(Widget* widget) override;

   private:
    Button* owner_;
  };

  void OnEnabledChanged();

  void WidgetActivationChanged(Widget* widget, bool active);

  // The text shown in a tooltip.
  cr::string16 tooltip_text_;

  // Accessibility data.
  cr::string16 accessible_name_;

  // The id tag associated with this button. Used to disambiguate buttons in
  // the ButtonListener implementation.
  int tag_ = -1;

  ButtonState state_ = STATE_NORMAL;

  gfx::ThrobAnimation hover_animation_{this};

  // Should we animate when the state changes?
  bool animate_on_state_change_ = false;

  // Is the hover animation running because StartThrob was invoked?
  bool is_throbbing_ = false;

  // Mouse event flags which can trigger button actions.
  int triggerable_event_flags_ = crui::EF_LEFT_MOUSE_BUTTON;

  // See description above setter.
  bool request_focus_on_press_ = false;

  // The focus ring for this Button.
  std::unique_ptr<FocusRing> focus_ring_;

  std::unique_ptr<Painter> focus_painter_;

  std::unique_ptr<WidgetObserverButtonBridge> widget_observer_;

  // ButtonController is responsible for handling events sent to the Button and
  // related state changes from the events.
  // TODO(cyan): Make sure all state changes are handled within
  // ButtonController.
  std::unique_ptr<ButtonController> button_controller_;

  PropertyChangedSubscription enabled_changed_subscription_{
      AddEnabledChangedCallback(cr::BindRepeating(&Button::OnEnabledChanged,
                                                  cr::Unretained(this)))};

  cr::ObserverList<ButtonObserver> button_observers_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_CONTROLS_BUTTON_BUTTON_H_

// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_BUTTON_BUTTON_CONTROLLER_H_
#define UI_VIEWS_CONTROLS_BUTTON_BUTTON_CONTROLLER_H_

#include "crui/events/event.h"
#include "crui/views/controls/button/button.h"

namespace crui {
namespace views {

// Handles logic not related to the visual aspects of a Button such as event
// handling and state changes.
class CRUI_EXPORT ButtonController {
 public:
  ButtonController(const ButtonController&) = delete;
  ButtonController& operator=(const ButtonController&) = delete;

  ButtonController(Button* button,
                   std::unique_ptr<ButtonControllerDelegate> delegate);
  virtual ~ButtonController();

  // An enum describing the events on which a button should notify its listener.
  enum class NotifyAction {
    kOnPress,
    kOnRelease,
  };

  Button* button() { return button_; }

  // Sets the event on which the button's listener should be notified.
  void set_notify_action(NotifyAction notify_action) {
    notify_action_ = notify_action;
  }

  NotifyAction notify_action() const { return notify_action_; }

  // Methods that parallel View::On<Event> handlers:
  virtual bool OnMousePressed(const crui::MouseEvent& event);
  virtual void OnMouseReleased(const crui::MouseEvent& event);
  virtual void OnMouseMoved(const crui::MouseEvent& event);
  virtual void OnMouseEntered(const crui::MouseEvent& event);
  virtual void OnMouseExited(const crui::MouseEvent& event);
  virtual bool OnKeyPressed(const crui::KeyEvent& event);
  virtual bool OnKeyReleased(const crui::KeyEvent& event);
  virtual void OnGestureEvent(crui::GestureEvent* event);

  // Updates |node_data| for a button based on the functionality.
  ///virtual void UpdateAccessibleNodeData(ui::AXNodeData* node_data);

  // Methods that parallel respective methods in Button:
  virtual void OnStateChanged(Button::ButtonState old_state);
  virtual bool IsTriggerableEvent(const crui::Event& event);

 protected:
  ButtonControllerDelegate* delegate() {
    return button_controller_delegate_.get();
  }

 private:
  Button* const button_;

  // TODO(cyan): Remove |button_| and access everything via the delegate.
  std::unique_ptr<ButtonControllerDelegate> button_controller_delegate_;

  // The event on which the button's listener should be notified.
  NotifyAction notify_action_ = NotifyAction::kOnRelease;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_CONTROLS_BUTTON_BUTTON_CONTROLLER_H_

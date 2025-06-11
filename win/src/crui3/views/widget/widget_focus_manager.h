// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_FOCUS_WIDGET_FOCUS_MANAGER_H_
#define UI_VIEWS_FOCUS_WIDGET_FOCUS_MANAGER_H_

#include "crbase/memory/no_destructor.h"
#include "crbase/observer_list.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

// This interface should be implemented by classes that want to be notified when
// the native focus is about to change.  Listeners implementing this interface
// will be invoked for all native focus changes across the entire Chrome
// application.  FocusChangeListeners are only called for changes within the
// children of a single top-level native-view.
class WidgetFocusChangeListener {
 public:
  virtual void OnNativeFocusChanged(gfx::NativeView focused_now) = 0;

 protected:
  virtual ~WidgetFocusChangeListener() = default;
};

class CRUI_EXPORT WidgetFocusManager {
 public:
  // Returns the singleton instance.
  static WidgetFocusManager* GetInstance();

  WidgetFocusManager(const WidgetFocusManager&) = delete;
  WidgetFocusManager& operator=(const WidgetFocusManager&) = delete;

  ~WidgetFocusManager();

  // Adds/removes a WidgetFocusChangeListener |listener| to the set of
  // active listeners.
  void AddFocusChangeListener(WidgetFocusChangeListener* listener);
  void RemoveFocusChangeListener(WidgetFocusChangeListener* listener);

  // Called when native-focus shifts within, or off, a widget. |focused_now| is
  // null when focus is lost.
  void OnNativeFocusChanged(gfx::NativeView focused_now);

  // Enable/Disable notification of registered listeners during calls
  // to OnNativeFocusChanged.  Used to prevent unwanted focus changes from
  // propagating notifications.
  void EnableNotifications() { enabled_ = true; }
  void DisableNotifications() { enabled_ = false; }

 private:
  class Owner;
  friend class cr::NoDestructor<WidgetFocusManager>;

  WidgetFocusManager();

  ///base::ObserverList<WidgetFocusChangeListener>::Unchecked
  cr::ObserverList<WidgetFocusChangeListener>
      focus_change_listeners_;

  bool enabled_ = true;
};

// A basic helper class that is used to disable native focus change
// notifications within a scope.
class CRUI_EXPORT AutoNativeNotificationDisabler {
 public:
   AutoNativeNotificationDisabler(
      const AutoNativeNotificationDisabler&) = delete;
   AutoNativeNotificationDisabler& operator=(
      const AutoNativeNotificationDisabler&) = delete;

  AutoNativeNotificationDisabler();
  ~AutoNativeNotificationDisabler();
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_FOCUS_WIDGET_FOCUS_MANAGER_H_

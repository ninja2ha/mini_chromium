// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_SESSION_CHANGE_OBSERVER_H_
#define UI_BASE_WIN_SESSION_CHANGE_OBSERVER_H_

#include <windows.h>

#include "crbase/functional/callback.h"
#include "crui/base/ui_export.h"

namespace crui {

// Calls the provided callback on WM_WTSSESSION_CHANGE messages along with
// managing the tricky business of observing a singleton object. Only
// WTS_SESSION_LOCK and WTS_SESSION_UNLOCK events trigger the callback
// because those are the only events existing observers handle.
class CRUI_EXPORT SessionChangeObserver {
 public:
  SessionChangeObserver(const SessionChangeObserver&) = delete;
  SessionChangeObserver& operator=(const SessionChangeObserver&) = delete;

  // WPARAM is the wparam passed to the OnWndProc when message is
  // WM_WTSSESSION_CHANGE. The bool indicates whether the session
  // change is for the current session or not. If we couldn't get the current
  // session id, it will be nullptr.
  using WtsCallback = cr::RepeatingCallback<void(WPARAM, const bool*)>;
  explicit SessionChangeObserver(const WtsCallback& callback);
  ~SessionChangeObserver();

 private:
  class WtsRegistrationNotificationManager;

  void OnSessionChange(WPARAM wparam, const bool* is_current_session);
  void ClearCallback();

  WtsCallback callback_;
};

}  // namespace crui

#endif  // UI_BASE_WIN_SESSION_CHANGE_OBSERVER_H_

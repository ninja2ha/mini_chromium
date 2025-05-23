// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/win/lock_state.h"

#include <windows.h>
#include <wtsapi32.h>

#include "crbase/functional/bind.h"
#include "crbase/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/win/windows_version.h"
#include "crui/base/win/session_change_observer.h"

namespace crui {

namespace {

// Checks if the current session is locked.
bool IsSessionLocked() {
  bool is_locked = false;
  LPWSTR buffer = nullptr;
  DWORD buffer_length = 0;
  if (::WTSQuerySessionInformationW(WTS_CURRENT_SERVER, WTS_CURRENT_SESSION,
                                    WTSSessionInfoEx, &buffer, 
                                    &buffer_length) &&
      buffer_length >= sizeof(WTSINFOEXW)) {
    auto* info = reinterpret_cast<WTSINFOEXW*>(buffer);
    auto session_flags = info->Data.WTSInfoExLevel1.SessionFlags;
    // For Windows 7 SessionFlags has inverted logic:
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ee621019.
    if (cr::win::GetVersion() == cr::win::Version::WIN7)
      is_locked = session_flags == WTS_SESSIONSTATE_UNLOCK;
    else
      is_locked = session_flags == WTS_SESSIONSTATE_LOCK;
  }
  if (buffer)
    ::WTSFreeMemory(buffer);
  return is_locked;
}

// Observes the screen lock state of Windows and caches the current state. This
// is necessary as IsSessionLocked uses WTSQuerySessionInformation internally,
// which is an expensive syscall and causes a performance regression as we query
// the current state quite often. http://crbug.com/940607.
class SessionLockedObserver {
 public:
  SessionLockedObserver(const SessionLockedObserver&) = delete;
  SessionLockedObserver& operator=(const SessionLockedObserver&) = delete;

  SessionLockedObserver()
      : session_change_observer_(
            cr::BindRepeating(&SessionLockedObserver::OnSessionChange,
                              cr::Unretained(this))),
        screen_locked_(IsSessionLocked()) {}

  bool IsLocked() const { return screen_locked_; }

 private:
  void OnSessionChange(WPARAM status_code, const bool* is_current_session) {
    if (is_current_session && !*is_current_session)
      return;
    if (status_code == WTS_SESSION_UNLOCK)
      screen_locked_ = false;
    else if (status_code == WTS_SESSION_LOCK && is_current_session)
      screen_locked_ = true;
  }
  SessionChangeObserver session_change_observer_;
  bool screen_locked_;
};

}  // namespace

bool IsWorkstationLocked() {
  static const cr::NoDestructor<SessionLockedObserver> observer;
  return observer->IsLocked();
}

}  // namespace crui

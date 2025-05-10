// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/win/session_change_observer.h"

#include <wtsapi32.h>

#include <memory>
#include <utility>

#include "crbase/functional/bind.h"
#include "crbase/functional/bind_helpers.h"
#include "crbase/functional/callback.h"
#include "crbase/location.h"
#include "crbase/memory/singleton.h"
#include "crbase/observer_list.h"
///#include "crbase/task/post_task.h"
#include "crbase/threading/worker_pool/worker_pool.h"
#include "crui/gfx/win/singleton_hwnd.h"
#include "crui/gfx/win/singleton_hwnd_observer.h"

namespace crui {

class SessionChangeObserver::WtsRegistrationNotificationManager {
 public:
  static WtsRegistrationNotificationManager* GetInstance() {
    return cr::Singleton<WtsRegistrationNotificationManager>::get();
  }

  WtsRegistrationNotificationManager(
      const WtsRegistrationNotificationManager&) = delete;
  WtsRegistrationNotificationManager& operator=(
      const WtsRegistrationNotificationManager&) = delete;

  WtsRegistrationNotificationManager() {
    CR_DCHECK(!singleton_hwnd_observer_);
    singleton_hwnd_observer_ = std::make_unique<gfx::SingletonHwndObserver>(
        cr::BindRepeating(&WtsRegistrationNotificationManager::OnWndProc,
                          cr::Unretained(this)));

    cr::OnceClosure wts_register = cr::BindOnce(
        cr::IgnoreResult(&WTSRegisterSessionNotification),
        gfx::SingletonHwnd::GetInstance()->hwnd(), NOTIFY_FOR_THIS_SESSION);

    ///cr::CreateCOMSTATaskRunner({base::ThreadPool()})
    ///    ->PostTask(FROM_HERE, std::move(wts_register));
    cr::WorkerPool::PostTask(CR_FROM_HERE, std::move(wts_register), false);
  }

  ~WtsRegistrationNotificationManager() { RemoveSingletonHwndObserver(); }

  void AddObserver(SessionChangeObserver* observer) {
    CR_DCHECK(singleton_hwnd_observer_);
    observer_list_.AddObserver(observer);
  }

  void RemoveObserver(SessionChangeObserver* observer) {
    observer_list_.RemoveObserver(observer);
  }

 private:
  friend struct cr::DefaultSingletonTraits<
      WtsRegistrationNotificationManager>;

  void OnWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
      case WM_WTSSESSION_CHANGE:
        if (wparam == WTS_SESSION_LOCK || wparam == WTS_SESSION_UNLOCK) {
          bool is_current_session;
          const bool* is_current_session_ptr = &is_current_session;
          DWORD current_session_id = 0;
          if (!::ProcessIdToSessionId(::GetCurrentProcessId(),
                                      &current_session_id)) {
            CR_PLOG(Error) << "ProcessIdToSessionId failed";
            is_current_session_ptr = nullptr;
          } else {
            is_current_session =
                (static_cast<DWORD>(lparam) == current_session_id);
          }
          for (SessionChangeObserver& observer : observer_list_)
            observer.OnSessionChange(wparam, is_current_session_ptr);
        }
        break;
      case WM_DESTROY:
        RemoveSingletonHwndObserver();
        break;
    }
  }

  void RemoveSingletonHwndObserver() {
    if (!singleton_hwnd_observer_)
      return;

    singleton_hwnd_observer_.reset(nullptr);
    // There is no race condition between this code and the worker thread.
    // RemoveSingletonHwndObserver is only called from two places:
    //   1) Destruction due to Singleton Destruction.
    //   2) WM_DESTROY fired by SingletonHwnd.
    // Under both cases we are in shutdown, which means no other worker threads
    // can be running.
    WTSUnRegisterSessionNotification(gfx::SingletonHwnd::GetInstance()->hwnd());
    for (SessionChangeObserver& observer : observer_list_)
      observer.ClearCallback();
  }

  ///cr::ObserverList<SessionChangeObserver, true>::Unchecked observer_list_;
  cr::ObserverList<SessionChangeObserver, true> observer_list_;
  std::unique_ptr<gfx::SingletonHwndObserver> singleton_hwnd_observer_;
};

SessionChangeObserver::SessionChangeObserver(const WtsCallback& callback)
    : callback_(callback) {
  CR_DCHECK(!callback_.is_null());
  WtsRegistrationNotificationManager::GetInstance()->AddObserver(this);
}

SessionChangeObserver::~SessionChangeObserver() {
  ClearCallback();
}

void SessionChangeObserver::OnSessionChange(WPARAM wparam,
                                            const bool* is_current_session) {
  callback_.Run(wparam, is_current_session);
}

void SessionChangeObserver::ClearCallback() {
  if (!callback_.is_null()) {
    callback_.Reset();
    WtsRegistrationNotificationManager::GetInstance()->RemoveObserver(this);
  }
}

}  // namespace crui

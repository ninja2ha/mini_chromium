// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_CAPTURE_CONTROLLER_H_
#define UI_WM_CORE_CAPTURE_CONTROLLER_H_

#include <map>

#include "crbase/observer_list.h"
#include "crui/aura/client/capture_client.h"
#include "crui/aura/window_observer.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace aura {
namespace client {
class CaptureDelegate;
}  // namespace client
}  // namespace aura

namespace wm {

// Internal CaptureClient implementation. See ScopedCaptureClient for details.
class CRUI_EXPORT CaptureController : public aura::client::CaptureClient {
 public:
  CaptureController(const CaptureController&) = delete;
  CaptureController& operator=(const CaptureController&) = delete;

  CaptureController();
  ~CaptureController() override;

  static CaptureController* Get() { return instance_; }

  // Adds |root| to the list of root windows notified when capture changes.
  void Attach(aura::Window* root);

  // Removes |root| from the list of root windows notified when capture changes.
  void Detach(aura::Window* root);

  // Returns true if this CaptureController is installed on at least one
  // root window.
  bool is_active() const { return !delegates_.empty(); }

  // Overridden from aura::client::CaptureClient:
  void SetCapture(aura::Window* window) override;
  void ReleaseCapture(aura::Window* window) override;
  aura::Window* GetCaptureWindow() override;
  aura::Window* GetGlobalCaptureWindow() override;
  void AddObserver(aura::client::CaptureClientObserver* observer) override;
  void RemoveObserver(aura::client::CaptureClientObserver* observer) override;

 private:
  friend class ScopedCaptureClient;

  static CaptureController* instance_;

  // The current capture window. NULL if there is no capture window.
  aura::Window* capture_window_;

  // The capture delegate for the root window with native capture. The root
  // window with native capture may not contain |capture_window_|. This occurs
  // if |capture_window_| is reparented to a different root window while it has
  // capture.
  aura::client::CaptureDelegate* capture_delegate_;

  // The delegates notified when capture changes.
  std::map<aura::Window*, aura::client::CaptureDelegate*> delegates_;

  ///cr::ObserverList<aura::client::CaptureClientObserver>::Unchecked observers_;
  cr::ObserverList<aura::client::CaptureClientObserver> observers_;
};

// ScopedCaptureClient is responsible for creating a CaptureClient for a
// RootWindow. Specifically it creates a single CaptureController that is shared
// among all ScopedCaptureClients and adds the RootWindow to it.
class CRUI_EXPORT ScopedCaptureClient : public aura::WindowObserver {
 public:
  class CRUI_EXPORT TestApi {
   public:
    TestApi(const TestApi&) = delete;
    TestApi& operator=(const TestApi&) = delete;

    explicit TestApi(ScopedCaptureClient* client) : client_(client) {}
    ~TestApi() {}

    // Sets the delegate.
    void SetDelegate(aura::client::CaptureDelegate* delegate);

   private:
    // Not owned.
    ScopedCaptureClient* client_;
  };

  ScopedCaptureClient(const ScopedCaptureClient&) = delete;
  ScopedCaptureClient& operator=(const ScopedCaptureClient&) = delete;

  explicit ScopedCaptureClient(aura::Window* root);
  ~ScopedCaptureClient() override;

  // Overridden from aura::WindowObserver:
  void OnWindowDestroyed(aura::Window* window) override;

 private:
  // Invoked from destructor and OnWindowDestroyed() to cleanup.
  void Shutdown();

  // RootWindow this ScopedCaptureClient was create for.
  aura::Window* root_window_;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_CAPTURE_CONTROLLER_H_

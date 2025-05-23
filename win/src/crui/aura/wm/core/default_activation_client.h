// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_DEFAULT_ACTIVATION_CLIENT_H_
#define UI_WM_CORE_DEFAULT_ACTIVATION_CLIENT_H_

#include <vector>

#include "crbase/compiler_specific.h"
#include "crbase/logging.h"
#include "crbase/observer_list.h"
#include "crui/aura/window_observer.h"
#include "crui/base/ui_export.h"
#include "crui/aura/wm/public/activation_change_observer.h"
#include "crui/aura/wm/public/activation_client.h"

namespace crui {
namespace wm {

class ActivationChangeObserver;

// Simple ActivationClient implementation for use by tests and other targets
// that just need basic behavior (e.g. activate windows whenever requested,
// restack windows at the top when they're activated, etc.). This object deletes
// itself when the root window it is associated with is destroyed.
class CRUI_EXPORT DefaultActivationClient : public ActivationClient,
                                            public aura::WindowObserver {
 public:
  explicit DefaultActivationClient(aura::Window* root_window);

  // Overridden from ActivationClient:
  void AddObserver(ActivationChangeObserver* observer) override;
  void RemoveObserver(ActivationChangeObserver* observer) override;
  void ActivateWindow(aura::Window* window) override;
  void DeactivateWindow(aura::Window* window) override;
  const aura::Window* GetActiveWindow() const override;
  aura::Window* GetActivatableWindow(aura::Window* window) const override;
  const aura::Window* GetToplevelWindow(
      const aura::Window* window) const override;
  bool CanActivateWindow(const aura::Window* window) const override;

  // Overridden from WindowObserver:
  void OnWindowDestroyed(aura::Window* window) override;

 private:
  class Deleter;

  ~DefaultActivationClient() override;
  void RemoveActiveWindow(aura::Window* window);

  void ActivateWindowImpl(ActivationChangeObserver::ActivationReason reason,
                          aura::Window* window);

  // This class explicitly does NOT store the active window in a window property
  // to make sure that ActivationChangeObserver is not treated as part of the
  // aura API. Assumptions to that end will cause tests that use this client to
  // fail.
  std::vector<aura::Window*> active_windows_;

  // The window which was active before the currently active one.
  aura::Window* last_active_;

  ///cr::ObserverList<ActivationChangeObserver>::Unchecked observers_;
  cr::ObserverList<ActivationChangeObserver> observers_;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_DEFAULT_ACTIVATION_CLIENT_H_

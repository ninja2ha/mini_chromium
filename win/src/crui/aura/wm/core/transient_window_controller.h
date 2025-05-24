// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_TRANSIENT_WINDOW_CONTROLLER_H_
#define UI_WM_CORE_TRANSIENT_WINDOW_CONTROLLER_H_

#include "crbase/observer_list.h"
#include "crui/aura/client/transient_window_client.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace wm {

class TransientWindowManager;

// TransientWindowClient implementation. Uses TransientWindowManager to handle
// tracking transient per window.
class CRUI_EXPORT TransientWindowController
    : public aura::client::TransientWindowClient {
 public:
  TransientWindowController(const TransientWindowController&) = delete;
  TransientWindowController& operator=(const TransientWindowController&) = delete;

  TransientWindowController();
  ~TransientWindowController() override;

  // Returns the single TransientWindowController instance.
  static TransientWindowController* Get() { return instance_; }

  // TransientWindowClient:
  void AddTransientChild(aura::Window* parent, aura::Window* child) override;
  void RemoveTransientChild(aura::Window* parent, aura::Window* child) override;
  aura::Window* GetTransientParent(aura::Window* window) override;
  const aura::Window* GetTransientParent(const aura::Window* window) override;
  std::vector<aura::Window*> GetTransientChildren(
      const aura::Window* parent) override;
  void AddObserver(
      aura::client::TransientWindowClientObserver* observer) override;
  void RemoveObserver(
      aura::client::TransientWindowClientObserver* observer) override;

 private:
  friend class TransientWindowManager;

  static TransientWindowController* instance_;

  ///cr::ObserverList<aura::client::TransientWindowClientObserver>::Unchecked
  cr::ObserverList<aura::client::TransientWindowClientObserver>
      observers_;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_TRANSIENT_WINDOW_CONTROLLER_H_

// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_TRANSIENT_WINDOW_STACKING_CLIENT_H_
#define UI_WM_CORE_TRANSIENT_WINDOW_STACKING_CLIENT_H_

#include "crui/aura/client/window_stacking_client.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace wm {

class TransientWindowManager;

class CRUI_EXPORT TransientWindowStackingClient
    : public aura::client::WindowStackingClient {
 public:
  TransientWindowStackingClient(
      const TransientWindowStackingClient&) = delete;
  TransientWindowStackingClient& operator=(
      const TransientWindowStackingClient&) = delete;

  TransientWindowStackingClient();
  ~TransientWindowStackingClient() override;

  // WindowStackingClient:
  bool AdjustStacking(aura::Window** child,
                      aura::Window** target,
                      aura::Window::StackDirection* direction) override;

 private:
  // Purely for DCHECKs.
  friend class TransientWindowManager;

  static TransientWindowStackingClient* instance_;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_TRANSIENT_WINDOW_STACKING_CLIENT_H_

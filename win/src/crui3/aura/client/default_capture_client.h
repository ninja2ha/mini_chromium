// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_DEFAULT_CAPTURE_CLIENT_H_
#define UI_AURA_CLIENT_DEFAULT_CAPTURE_CLIENT_H_

#include "crbase/observer_list.h"
#include "crui/base/ui_export.h"
#include "crui/aura/client/capture_client.h"

namespace crui {

namespace aura {
namespace client {

class CRUI_EXPORT DefaultCaptureClient : public client::CaptureClient {
 public:
  DefaultCaptureClient(const DefaultCaptureClient&) = delete;
  DefaultCaptureClient& operator=(const DefaultCaptureClient&) = delete;

  explicit DefaultCaptureClient(Window* root_window = nullptr);
  ~DefaultCaptureClient() override;

 protected:
  // Overridden from client::CaptureClient:
  void SetCapture(Window* window) override;
  void ReleaseCapture(Window* window) override;
  Window* GetCaptureWindow() override;
  Window* GetGlobalCaptureWindow() override;
  void AddObserver(CaptureClientObserver* observer) override;
  void RemoveObserver(CaptureClientObserver* observer) override;

 private:
  Window* root_window_;  // May be null.
  Window* capture_window_;
  ///cr::ObserverList<CaptureClientObserver>::Unchecked observers_;
  cr::ObserverList<CaptureClientObserver> observers_;
};

}  // namespace client
}  // namespace aura

}  // namespace crui

#endif  // UI_AURA_CLIENT_DEFAULT_CAPTURE_CLIENT_H_

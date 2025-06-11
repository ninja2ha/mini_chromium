// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_CAPTURE_CLIENT_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_CAPTURE_CLIENT_H_

#include <set>

#include "crbase/compiler_specific.h"
#include "crbase/observer_list.h"
#include "crui/aura/client/capture_client.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace aura {
class RootWindow;
}  // namespace aura

namespace views {

// Desktop implementation of CaptureClient. There is one CaptureClient per
// DesktopNativeWidgetAura.
//
// DesktopCaptureClient and CaptureController (used by ash) differ slightly in
// how they handle capture. CaptureController is a singleton shared among all
// RootWindows created by ash. An implication of this is that all RootWindows
// know which window has capture. This is not the case with
// DesktopCaptureClient. Instead each RootWindow has its own
// DesktopCaptureClient. This means only the RootWindow of the Window that has
// capture knows which window has capture. All others think no one has
// capture. This behavior is necessitated by Windows occassionally delivering
// mouse events to a window other than the capture window and expecting that
// window to get the event. If we shared the capture window on the desktop this
// behavior would not be possible.
class CRUI_EXPORT DesktopCaptureClient : public aura::client::CaptureClient {
 public:
  DesktopCaptureClient(const DesktopCaptureClient&) = delete;
  DesktopCaptureClient& operator=(const DesktopCaptureClient&) = delete;

  explicit DesktopCaptureClient(aura::Window* root);
  ~DesktopCaptureClient() override;

  // Exactly the same as GetGlobalCaptureWindow() but static.
  static aura::Window* GetCaptureWindowGlobal();

  // Overridden from aura::client::CaptureClient:
  void SetCapture(aura::Window* window) override;
  void ReleaseCapture(aura::Window* window) override;
  aura::Window* GetCaptureWindow() override;
  aura::Window* GetGlobalCaptureWindow() override;
  void AddObserver(aura::client::CaptureClientObserver* observer) override;
  void RemoveObserver(aura::client::CaptureClientObserver* observer) override;

 private:
  using Comparator = bool (*)(const cr::WeakPtr<DesktopCaptureClient>&,
                              const cr::WeakPtr<DesktopCaptureClient>&);
  using ClientSet = std::set<cr::WeakPtr<DesktopCaptureClient>, Comparator>;

  aura::Window* root_;
  aura::Window* capture_window_ = nullptr;

  // The global set of DesktopCaptureClients.
  static ClientSet* clients_;

  ///cr::ObserverList<aura::client::CaptureClientObserver>::Unchecked observers_;
  cr::ObserverList<aura::client::CaptureClientObserver> observers_;

  cr::WeakPtrFactory<DesktopCaptureClient> weak_factory_{this};
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_CAPTURE_CLIENT_H_

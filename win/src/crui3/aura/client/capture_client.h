// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_CAPTURE_CLIENT_H_
#define UI_AURA_CLIENT_CAPTURE_CLIENT_H_

#include "crui/base/ui_export.h"

namespace crui {

namespace aura {
class Window;

namespace client {

class CaptureClientObserver;

// An interface implemented by an object that manages input capture.
class CRUI_EXPORT CaptureClient {
 public:
  // Does a capture on the |window|.
  virtual void SetCapture(Window* window) = 0;

  // Releases a capture from the |window|.
  virtual void ReleaseCapture(Window* window) = 0;

  // Returns the current capture window. This may only return a Window if the
  // Window that has capture is a child of the Window the CaptureClient is
  // installed on. GetGlobalCaptureWindow() can be used to locate the Window
  // that has capture regardless of the Window the CaptureClient is installed
  // on.
  virtual Window* GetCaptureWindow() = 0;

  // See description of GetCaptureWindow() for details.
  virtual Window* GetGlobalCaptureWindow() = 0;

  virtual void AddObserver(CaptureClientObserver* observer) = 0;
  virtual void RemoveObserver(CaptureClientObserver* observer) = 0;

 protected:
  virtual ~CaptureClient() {}
};

// Sets/Gets the capture client on the root Window.
CRUI_EXPORT void SetCaptureClient(Window* root_window,
                                  CaptureClient* client);
CRUI_EXPORT CaptureClient* GetCaptureClient(Window* root_window);

// A utility function to get the current capture window. Returns NULL
// if the window doesn't have a root window, or there is no capture window.
CRUI_EXPORT Window* GetCaptureWindow(Window* window);

}  // namespace client
}  // namespace aura

}  // namespace crui

#endif  // UI_AURA_CLIENT_CAPTURE_CLIENT_H_

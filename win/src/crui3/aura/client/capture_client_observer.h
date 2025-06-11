// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_CAPTURE_CLIENT_OBSERVER_H_
#define UI_AURA_CLIENT_CAPTURE_CLIENT_OBSERVER_H_

#include "crui/base/ui_export.h"

namespace crui {

namespace aura {
class Window;

namespace client {

// Used to observe changes in capture.
class CRUI_EXPORT CaptureClientObserver {
 public:
  virtual void OnCaptureChanged(Window* lost_capture,
                                Window* gained_capture) = 0;

 protected:
  virtual ~CaptureClientObserver() {}
};

}  // namespace client
}  // namespace aura

}  // namespace crui

#endif  // UI_AURA_CLIENT_CAPTURE_CLIENT_OBSERVER_H_

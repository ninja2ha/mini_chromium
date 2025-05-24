// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CLIPBOARD_CLIPBOARD_OBSERVER_H_
#define UI_BASE_CLIPBOARD_CLIPBOARD_OBSERVER_H_

#include "crui/base/ui_export.h"

namespace crui {

// Observer that receives the notifications of clipboard change events.
class CRUI_EXPORT ClipboardObserver {
 public:
  // Called when clipboard data is changed.
  virtual void OnClipboardDataChanged() = 0;

 protected:
  virtual ~ClipboardObserver() = default;
};

}  // namespace crui

#endif  // UI_BASE_CLIPBOARD_CLIPBOARD_OBSERVER_H_

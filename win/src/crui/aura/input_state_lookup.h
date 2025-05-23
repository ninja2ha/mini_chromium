// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_INPUT_STATE_LOOKUP_H_
#define UI_AURA_INPUT_STATE_LOOKUP_H_

#include <memory>

#include "crui/base/ui_export.h"

namespace crui {
namespace aura {

// InputStateLookup is used to obtain the state of input devices.
class CRUI_EXPORT InputStateLookup {
 public:
  virtual ~InputStateLookup() {}

  // Creates the platform specific InputStateLookup. May return NULL.
  static std::unique_ptr<InputStateLookup> Create();

  // Returns true if any mouse button is down.
  virtual bool IsMouseButtonDown() const = 0;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_INPUT_STATE_LOOKUP_H_

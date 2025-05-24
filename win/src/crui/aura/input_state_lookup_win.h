// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_INPUT_STATE_LOOKUP_WIN_H_
#define UI_AURA_INPUT_STATE_LOOKUP_WIN_H_

#include "crbase/compiler_specific.h"
#include "crui/aura/input_state_lookup.h"

namespace crui {
namespace aura {

// Windows implementation of InputStateLookup.
class CRUI_EXPORT InputStateLookupWin : public InputStateLookup {
 public:
  InputStateLookupWin(const InputStateLookupWin&) = delete;
  InputStateLookupWin& operator=(const InputStateLookupWin&) = delete;

  InputStateLookupWin();
  ~InputStateLookupWin() override;

  // InputStateLookup overrides:
  bool IsMouseButtonDown() const override;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_INPUT_STATE_LOOKUP_WIN_H_

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/input_state_lookup_win.h"

#include <windows.h>
#include <winuser.h>

#include "crbase/memory/ptr_util.h"

namespace crui {
namespace aura {

// static
std::unique_ptr<InputStateLookup> InputStateLookup::Create() {
  return cr::WrapUnique(new InputStateLookupWin);
}

InputStateLookupWin::InputStateLookupWin() {
}

InputStateLookupWin::~InputStateLookupWin() {
}

bool InputStateLookupWin::IsMouseButtonDown() const {
  return (GetKeyState(VK_LBUTTON) & 0x80) ||
    (GetKeyState(VK_RBUTTON) & 0x80) ||
    (GetKeyState(VK_MBUTTON) & 0x80) ||
    (GetKeyState(VK_XBUTTON1) & 0x80) ||
    (GetKeyState(VK_XBUTTON2) & 0x80);
}

}  // namespace aura
}  // namespace crui

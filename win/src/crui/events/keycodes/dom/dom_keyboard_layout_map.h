// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYCODES_DOM_DOM_KEYBOARD_LAYOUT_MAP_H_
#define UI_EVENTS_KEYCODES_DOM_DOM_KEYBOARD_LAYOUT_MAP_H_

#include <string>

#include "crbase/containers/flat_map.h"

namespace crui {

// Generates a map representing the physical keys on the current keyboard
// layout.  Each entry maps a DomCode string to a DomKey string.  The current
// layout is determined by the underlying OS platform which may be the active
// layout or the first ASCII capable layout available.
// More info at: https://wicg.github.io/keyboard-map/
cr::flat_map<std::string, std::string> GenerateDomKeyboardLayoutMap();

}  // namespace crui

#endif  // UI_EVENTS_KEYCODES_DOM_DOM_KEYBOARD_LAYOUT_MAP_H_

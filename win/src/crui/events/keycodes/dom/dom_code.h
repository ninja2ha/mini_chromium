// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYCODES_DOM3_DOM_CODE_H_
#define UI_EVENTS_KEYCODES_DOM3_DOM_CODE_H_

namespace crui {

#define DOM_CODE(usb, evdev, xkb, win, mac, code, id) id = usb
#define DOM_CODE_DECLARATION enum class DomCode
#include "crui/events/keycodes/dom/dom_code_data.inc"
#undef DOM_CODE
#undef DOM_CODE_DECLARATION

}  // namespace crui

#endif  // UI_EVENTS_KEYCODES_DOM3_DOM_CODE_H_

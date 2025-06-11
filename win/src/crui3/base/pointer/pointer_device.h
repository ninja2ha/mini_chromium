// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_POINTER_POINTER_DEVICE_H_
#define UI_BASE_POINTER_POINTER_DEVICE_H_

#include <tuple>

#include "crui/base/ui_export.h"
#include "crui/base/build_platform.h"

namespace crui {

enum class TouchScreensAvailability {
  NONE,      // No touch screens are present.
  ENABLED,   // Touch screens are present and enabled.
  DISABLED,  // Touch screens are present and disabled.
};

CRUI_EXPORT TouchScreensAvailability GetTouchScreensAvailability();

// Returns the maximum number of simultaneous touch contacts supported
// by the device. In the case of devices with multiple digitizers (e.g.
// multiple touchscreens), the value MUST be the maximum of the set of
// maximum supported contacts by each individual digitizer.
// For example, suppose a device has 3 touchscreens, which support 2, 5,
// and 10 simultaneous touch contacts, respectively. This returns 10.
// http://www.w3.org/TR/pointerevents/#widl-Navigator-maxTouchPoints
CRUI_EXPORT int MaxTouchPoints();

// Bit field values indicating available pointer types. Identical to
// blink::PointerType enums, enforced by compile-time assertions in
// content/public/common/web_preferences.cc .
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.ui.base
// GENERATED_JAVA_PREFIX_TO_STRIP: POINTER_TYPE_
enum PointerType {
  POINTER_TYPE_NONE = 1 << 0,
  POINTER_TYPE_FIRST = POINTER_TYPE_NONE,
  POINTER_TYPE_COARSE = 1 << 1,
  POINTER_TYPE_FINE = 1 << 2,
  POINTER_TYPE_LAST = POINTER_TYPE_FINE
};

// Bit field values indicating available hover types. Identical to
// blink::HoverType enums, enforced by compile-time assertions in
// content/public/common/web_preferences.cc .
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.ui.base
// GENERATED_JAVA_PREFIX_TO_STRIP: HOVER_TYPE_
enum HoverType {
  HOVER_TYPE_NONE = 1 << 0,
  HOVER_TYPE_FIRST = HOVER_TYPE_NONE,
  HOVER_TYPE_HOVER = 1 << 1,
  HOVER_TYPE_LAST = HOVER_TYPE_HOVER
};

int GetAvailablePointerTypes();
int GetAvailableHoverTypes();
CRUI_EXPORT std::pair<int, int> GetAvailablePointerAndHoverTypes();
CRUI_EXPORT PointerType GetPrimaryPointerType(int available_pointer_types);
CRUI_EXPORT HoverType GetPrimaryHoverType(int available_hover_types);

}  // namespace crui

#endif  // UI_BASE_POINTER_POINTER_DEVICE_H_

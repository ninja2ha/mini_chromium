// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_GESTURE_PROVIDER_CONFIG_HELPER_H_
#define UI_EVENTS_GESTURE_DETECTION_GESTURE_PROVIDER_CONFIG_HELPER_H_

#include "crui/events/gesture_detection/gesture_detector.h"
#include "crui/events/gesture_detection/gesture_provider.h"
#include "crui/events/gesture_detection/scale_gesture_detector.h"
#include "crui/base/ui_export.h"

namespace crui {

enum class GestureProviderConfigType {
  CURRENT_PLATFORM,     // Parameters tailored for the current platform.
  CURRENT_PLATFORM_VR,  // Parameters tailored for the current platform in VR.
  GENERIC_DESKTOP,      // Parameters typical for a desktop machine.
  GENERIC_MOBILE  // Parameters typical for a mobile device (phone/tablet).
};

CRUI_EXPORT GestureProvider::Config GetGestureProviderConfig(
    GestureProviderConfigType);

}  // namespace crui

#endif  // UI_EVENTS_GESTURE_DETECTION_GESTURE_PROVIDER_CONFIG_HELPER_H_

// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/display/win/dpi.h"

#include <windows.h>

#include "crui/base/win/scoped_hdc.h"
#include "crui/display/display.h"
///#include "crui/display/win/uwp_text_scale_factor.h"

namespace crui {
namespace display {
namespace win {

namespace {

const float kDefaultDPI = 96.f;

float g_device_scale_factor = 0.f;

}  // namespace

void SetDefaultDeviceScaleFactor(float scale) {
  CR_DCHECK(0.f != scale);
  g_device_scale_factor = scale;
}

float GetDPIScale() {
  if (Display::HasForceDeviceScaleFactor())
    return Display::GetForcedDeviceScaleFactor();
  return display::win::internal::GetUnforcedDeviceScaleFactor();
}

int GetDPIFromScalingFactor(float device_scaling_factor) {
  return static_cast<int>(kDefaultDPI * device_scaling_factor);
}

double GetAccessibilityFontScale() {
  return 1.0;/// / UwpTextScaleFactor::Instance()->GetTextScaleFactor();
}

namespace internal {

int GetDefaultSystemDPI() {
  static int dpi_x = 0;
  static int dpi_y = 0;
  static bool should_initialize = true;

  if (should_initialize) {
    should_initialize = false;
    crui::win::ScopedGetDC screen_dc(NULL);
    // This value is safe to cache for the life time of the app since the
    // user must logout to change the DPI setting. This value also applies
    // to all screens.
    dpi_x = GetDeviceCaps(screen_dc, LOGPIXELSX);
    dpi_y = GetDeviceCaps(screen_dc, LOGPIXELSY);
    CR_DCHECK(dpi_x == dpi_y);
  }
  return dpi_x;
}

float GetUnforcedDeviceScaleFactor() {
  return g_device_scale_factor ? g_device_scale_factor
                               : GetScalingFactorFromDPI(GetDefaultSystemDPI());
}

float GetScalingFactorFromDPI(int dpi) {
  return static_cast<float>(dpi) / kDefaultDPI;
}

}  // namespace internal
}  // namespace win
}  // namespace display
}  // namespace crui

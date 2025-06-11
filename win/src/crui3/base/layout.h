// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_LAYOUT_H_
#define UI_BASE_LAYOUT_H_

#include <vector>

#include "crui/base/resource/scale_factor.h"
#include "crui/base/ui_export.h"
#include "crui/gfx/native_widget_types.h"

namespace crui {

// Changes the value of GetSupportedScaleFactors() to |scale_factors|.
// Use ScopedSetSupportedScaleFactors for unit tests as not to affect the
// state of other tests.
CRUI_EXPORT void SetSupportedScaleFactors(
    const std::vector<ScaleFactor>& scale_factors);

// Returns a vector with the scale factors which are supported by this
// platform, in ascending order.
CRUI_EXPORT const std::vector<ScaleFactor>& GetSupportedScaleFactors();

// Returns the supported ScaleFactor which most closely matches |scale|.
// Converting from float to ScaleFactor is inefficient and should be done as
// little as possible.
CRUI_EXPORT ScaleFactor GetSupportedScaleFactor(float image_scale);

// Returns the ScaleFactor used by |view|.
CRUI_EXPORT float GetScaleFactorForNativeView(gfx::NativeView view);

// Returns true if the scale passed in is the list of supported scales for
// the platform.
CRUI_EXPORT bool IsSupportedScale(float scale);

}  // namespace crui

#endif  // UI_BASE_LAYOUT_H_

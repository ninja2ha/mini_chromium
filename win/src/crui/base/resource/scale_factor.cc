// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/resource/scale_factor.h"

#include "crbase/helper/stl_util.h"

namespace crui {

namespace {

const float kScaleFactorScales[] = {1.0f, 1.0f, 1.25f, 1.33f, 1.4f, 1.5f, 1.8f,
                                    2.0f, 2.5f, 3.0f};
static_assert(NUM_SCALE_FACTORS == cr::size(kScaleFactorScales),
              "kScaleFactorScales has incorrect size");

}  // namespace

float GetScaleForScaleFactor(ScaleFactor scale_factor) {
  return kScaleFactorScales[scale_factor];
}

}  // namespace crui

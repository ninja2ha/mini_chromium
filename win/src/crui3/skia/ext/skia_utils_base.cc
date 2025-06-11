// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "crui/skia/ext/skia_utils_base.h"
#include "third_party/skia/include/core/SkFontHost.h"
#include "third_party/skia/include/config/SkUserConfig.h"

namespace skia {

SkPixelGeometry ComputeDefaultPixelGeometry() {
    SkFontHost::LCDOrder order = SkFontHost::GetSubpixelOrder();
    if (SkFontHost::kNONE_LCDOrder == order) {
        return kUnknown_SkPixelGeometry;
    } else {
        // Bit0 is RGB(0), BGR(1)
        // Bit1 is H(0), V(1)
        const SkPixelGeometry gGeo[] = {
            kRGB_H_SkPixelGeometry,
            kBGR_H_SkPixelGeometry,
            kRGB_V_SkPixelGeometry,
            kBGR_V_SkPixelGeometry,
        };
        int index = 0;
        if (SkFontHost::kBGR_LCDOrder == order) {
            index |= 1;
        }
        if (SkFontHost::kVertical_LCDOrientation == SkFontHost::GetSubpixelOrientation()){
            index |= 2;
        }
        return gGeo[index];
    }
}

}  // namespace skia


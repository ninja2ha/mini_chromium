/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "src/core/SkBlitMask.h"
#include "src/core/SkUtilsArm.h"
#include "src/opts/SkBlitMask_opts_arm_neon.h"

SkBlitMask::BlitLCD16RowProc SkBlitMask::PlatformBlitRowProcs16(bool isOpaque) {
    if (isOpaque) {
        return SK_ARM_NEON_WRAP(SkBlitLCD16OpaqueRow);
    } else {
        return SK_ARM_NEON_WRAP(SkBlitLCD16Row);
    }
}

SkBlitMask::RowProc SkBlitMask::PlatformRowProcs(SkColorType dstCT,
                                                 SkMask::Format maskFormat,
                                                 RowFlags flags) {
    return nullptr;
}

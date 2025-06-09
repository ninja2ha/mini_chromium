/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/core/SkBitmapScaler.h"
#include "src/core/SkBitmapProcState.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTypes.h"
#include "include/core/SkUtils.h"
#include "src/core/SkUtilsArm.h"

#include "src/core/SkConvolver.h"

void SkBitmapProcState::platformProcs() { }

///////////////////////////////////////////////////////////////////////////////

extern void platformConvolutionProcs_arm_neon(SkConvolutionProcs* procs);

void platformConvolutionProcs_arm(SkConvolutionProcs* procs) {
}

void SkBitmapScaler::PlatformConvolutionProcs(SkConvolutionProcs* procs) {
    SK_ARM_NEON_WRAP(platformConvolutionProcs_arm)(procs);
}

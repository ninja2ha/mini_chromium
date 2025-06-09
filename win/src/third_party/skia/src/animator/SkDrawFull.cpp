
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/animator/SkDrawFull.h"
#include "src/animator/SkAnimateMaker.h"
#include "include/core/SkCanvas.h"

bool SkFull::draw(SkAnimateMaker& maker) {
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawPaint(*maker.fPaint);
    return false;
}

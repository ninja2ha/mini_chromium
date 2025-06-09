
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/animator/SkTextOnPath.h"
#include "src/animator/SkAnimateMaker.h"
#include "include/core/SkCanvas.h"
#include "src/animator/SkDrawPath.h"
#include "src/animator/SkDrawText.h"
#include "include/core/SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkTextOnPath::fInfo[] = {
    SK_MEMBER(offset, Float),
    SK_MEMBER(path, Path),
    SK_MEMBER(text, Text)
};

#endif

DEFINE_GET_MEMBER(SkTextOnPath);

SkTextOnPath::SkTextOnPath() : offset(0), path(nullptr), text(nullptr) {
}

bool SkTextOnPath::draw(SkAnimateMaker& maker) {
    SkASSERT(text);
    SkASSERT(path);
    SkBoundableAuto boundable(this, maker);
    maker.fCanvas->drawTextOnPathHV(text->getText(), text->getSize(),
                                    path->getPath(), offset, 0, *maker.fPaint);
    return false;
}

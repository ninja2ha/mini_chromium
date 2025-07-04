
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/animator/SkTextToPath.h"
#include "src/animator/SkAnimateMaker.h"
#include "src/animator/SkDrawPaint.h"
#include "src/animator/SkDrawPath.h"
#include "src/animator/SkDrawText.h"
#include "include/core/SkPaint.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkTextToPath::fInfo[] = {
    SK_MEMBER(paint, Paint),
    SK_MEMBER(path, Path),
    SK_MEMBER(text, Text)
};

#endif

DEFINE_GET_MEMBER(SkTextToPath);

SkTextToPath::SkTextToPath() : paint(nullptr), path(nullptr), text(nullptr) {
}

bool SkTextToPath::draw(SkAnimateMaker& maker) {
    path->draw(maker);
    return false;
}

void SkTextToPath::onEndElement(SkAnimateMaker& maker) {
    if (paint == nullptr || path == nullptr || text == nullptr) {
        // !!! add error message here
        maker.setErrorCode(SkDisplayXMLParserError::kErrorInAttributeValue);
        return;
    }
    SkPaint realPaint;
    paint->setupPaint(&realPaint);
    realPaint.getTextPath(text->getText(), text->getSize(), text->x,
        text->y, &path->getPath());
}

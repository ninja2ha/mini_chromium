
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/svg/parser/SkSVGMask.h"
#include "include/svg/parser/SkSVGParser.h"

const SkSVGAttribute SkSVGMask::gAttributes[] = {
    SVG_ATTRIBUTE(height),
    SVG_ATTRIBUTE(maskUnits),
    SVG_ATTRIBUTE(width),
    SVG_ATTRIBUTE(x),
    SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Mask)

bool SkSVGMask::isDef() {
    return false;
}

bool SkSVGMask::isNotDef() {
    return false;
}

void SkSVGMask::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
}

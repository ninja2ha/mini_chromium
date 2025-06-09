
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/svg/parser/SkSVGGroup.h"
#include "include/svg/parser/SkSVGParser.h"

SkSVGGroup::SkSVGGroup() {
    fIsNotDef = false;
}

SkSVGElement* SkSVGGroup::getGradient() {
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++) {
        SkSVGElement* result = (*ptr)->getGradient();
        if (result != nullptr)
            return result;
    }
    return nullptr;
}

bool SkSVGGroup::isDef() {
    return fParent ? fParent->isDef() : false;
}

bool SkSVGGroup::isFlushable() {
    return false;
}

bool SkSVGGroup::isGroup() {
    return true;
}

bool SkSVGGroup::isNotDef() {
    return fParent ? fParent->isNotDef() : false;
}

void SkSVGGroup::translate(SkSVGParser& parser, bool defState) {
    for (SkSVGElement** ptr = fChildren.begin(); ptr < fChildren.end(); ptr++)
        parser.translate(*ptr, defState);
}

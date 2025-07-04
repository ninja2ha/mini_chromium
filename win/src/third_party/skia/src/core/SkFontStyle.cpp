/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"

SkFontStyle::SkFontStyle() {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = kNormal_Weight;
    fUnion.fR.fWidth = kNormal_Width;
    fUnion.fR.fSlant = kUpright_Slant;
}

SkFontStyle::SkFontStyle(int weight, int width, Slant slant) {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = SkTPin<int>(weight, kThin_Weight, kBlack_Weight);
    fUnion.fR.fWidth = SkTPin<int>(width, kUltraCondensed_Width, kUltaExpanded_Width);
    fUnion.fR.fSlant = SkTPin<int>(slant, kUpright_Slant, kItalic_Slant);
}

SkFontStyle::SkFontStyle(unsigned oldStyle) {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = (oldStyle & SkTypeface::kBold) ? SkFontStyle::kBold_Weight
                                                       : SkFontStyle::kNormal_Weight;
    fUnion.fR.fWidth = SkFontStyle::kNormal_Width;
    fUnion.fR.fSlant = (oldStyle & SkTypeface::kItalic) ? SkFontStyle::kItalic_Slant
                                                        : SkFontStyle::kUpright_Slant;
}

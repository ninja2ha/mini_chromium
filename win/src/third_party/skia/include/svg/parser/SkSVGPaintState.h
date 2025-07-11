
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSVGPaintState_DEFINED
#define SkSVGPaintState_DEFINED

#include "include/svg/parser/SkSVGBase.h"
#include "include/core/SkString.h"

class SkSVGPaint : public SkSVGBase {
public:
    enum Field {
        kInitial = -1,
        kClipPath,
        kClipRule,
        kEnableBackground,
        kFill,
        kFillRule,
        kFilter,
        kFontFamily,
        kFontSize,
        kLetterSpacing,
        kMask,
        kOpacity,
        kStopColor,
        kStopOpacity,
        kStroke,
        kStroke_Dasharray,
        kStroke_Linecap,
        kStroke_Linejoin,
        kStroke_Miterlimit,
        kStroke_Width,
        kStyle,
        kTransform,
        kTerminal
    };

    SkSVGPaint();
    virtual void addAttribute(SkSVGParser& parser, int attrIndex,
        const char* attrValue, size_t attrLength);
    bool flush(SkSVGParser& , bool isFlushable, bool isDef);
    virtual int getAttributes(const SkSVGAttribute** attrPtr);
    static void Push(SkSVGPaint** head, SkSVGPaint* add);
    static void Pop(SkSVGPaint** head);
    SkString* operator[](int index);
    SkString fInitial;
    SkString f_clipPath;
    SkString f_clipRule;
    SkString f_enableBackground;
    SkString f_fill;
    SkString f_fillRule;
    SkString f_filter;
    SkString f_fontFamily;
    SkString f_fontSize;
    SkString f_letterSpacing;
    SkString f_mask;
    SkString f_opacity;
    SkString f_stopColor;
    SkString f_stopOpacity;
    SkString f_stroke;
    SkString f_strokeDasharray;
    SkString f_strokeLinecap;
    SkString f_strokeLinejoin;
    SkString f_strokeMiterlimit;
    SkString f_strokeWidth;
    SkString f_style; // unused, but allows array access to the rest
    SkString f_transform;
#ifdef SK_DEBUG
    SkString fTerminal;
#endif
    SkString fTransformID;
    static SkSVGAttribute gAttributes[];
    static const int kAttributesSize;
private:
    void setSave(SkSVGParser& );
    bool writeChangedAttributes(SkSVGParser& , SkSVGPaint& , bool* changed);
    bool writeChangedElements(SkSVGParser& , SkSVGPaint& , bool* changed);
    SkSVGPaint* fNext;
    friend class SkSVGParser;
    typedef SkSVGPaint BASE_CLASS;
};

#endif // SkSVGPaintState_DEFINED

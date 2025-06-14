/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintPart_DEFINED
#define SkPaintPart_DEFINED

#include "src/animator/SkDisplayable.h"
#include "src/animator/SkMemberInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkXfermode.h"

class SkDrawPaint;
class SkDrawMatrix;

class SkPaintPart : public SkDisplayable {
public:
    SkPaintPart();
    virtual bool add() = 0;
    virtual SkDisplayable* getParent() const;
    virtual bool setParent(SkDisplayable* parent);
#ifdef SK_DEBUG
    virtual bool isPaintPart() const { return true; }
#endif
protected:
    SkDrawPaint* fPaint;
};

class SkDrawMaskFilter : public SkPaintPart {
    DECLARE_EMPTY_MEMBER_INFO(MaskFilter);
    virtual SkMaskFilter* getMaskFilter();
protected:
    bool add() override;
};

class SkDrawPathEffect : public SkPaintPart {
    DECLARE_EMPTY_MEMBER_INFO(PathEffect);
    virtual SkPathEffect* getPathEffect();
protected:
    bool add() override;
};

class SkDrawShader : public SkPaintPart {
    DECLARE_DRAW_MEMBER_INFO(Shader);
    SkDrawShader();
    virtual SkShader* getShader();
protected:
    bool add() override;
    SkMatrix* getMatrix(); // returns nullptr if matrix is nullptr
    SkDrawMatrix* matrix;
    int /*SkShader::TileMode*/ tileMode;
};

class SkDrawTypeface  : public SkPaintPart {
    DECLARE_DRAW_MEMBER_INFO(Typeface);
    SkDrawTypeface();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker *) override;
#endif
    SkTypeface* getTypeface() {
        return SkTypeface::CreateFromName(fontName.c_str(), style); }
protected:
    bool add() override;
    SkString fontName;
    SkTypeface::Style style;
};

#endif // SkPaintPart_DEFINED


/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/views/SkWidget.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMath.h"
#include "include/core/SkShader.h"
#include "include/utils/SkInterpolator.h"
#include "include/core/SkTime.h"

SkProgressView::SkProgressView(uint32_t flags) : SkView(flags), fOnShader(nullptr), fOffShader(nullptr)
{
    fValue = 0;
    fMax = 0;
    fInterp = nullptr;
    fDoInterp = false;
}

SkProgressView::~SkProgressView()
{
    delete fInterp;
    SkSafeUnref(fOnShader);
    SkSafeUnref(fOffShader);
}

void SkProgressView::setMax(U16CPU max)
{
    if (fMax != max)
    {
        fMax = SkToU16(max);
        if (fValue > 0)
            this->inval(nullptr);
    }
}

void SkProgressView::setValue(U16CPU value)
{
    if (fValue != value)
    {
        if (fDoInterp)
        {
            if (fInterp)
                delete fInterp;
            fInterp = new SkInterpolator(1, 2);
            SkScalar x = (SkScalar)(fValue << 8);
            fInterp->setKeyFrame(0, SkTime::GetMSecs(), &x, 0);
            x = (SkScalar)(value << 8);
            fInterp->setKeyFrame(1, SkTime::GetMSecs() + 333, &x);
        }
        fValue = SkToU16(value);
        this->inval(nullptr);
    }
}

void SkProgressView::onDraw(SkCanvas* canvas)
{
    if (fMax == 0)
        return;

    SkFixed    percent;

    if (fInterp)
    {
        SkScalar x;
        if (fInterp->timeToValues(SkTime::GetMSecs(), &x) == SkInterpolator::kFreezeEnd_Result)
        {
            delete fInterp;
            fInterp = nullptr;
        }
        percent = (SkFixed)x;    // now its 16.8
        percent = SkMax32(0, SkMin32(percent, fMax << 8));    // now its pinned
        percent = SkFixedDiv(percent, fMax << 8);    // now its 0.16
        this->inval(nullptr);
    }
    else
    {
        U16CPU value = SkMax32(0, SkMin32(fValue, fMax));
        percent = SkFixedDiv(value, fMax);
    }


    SkRect    r;
    SkPaint    p;

    r.set(0, 0, this->width(), this->height());
    p.setAntiAlias(true);

    r.fRight = r.fLeft + SkScalarMul(r.width(), SkFixedToScalar(percent));
    p.setStyle(SkPaint::kFill_Style);

    p.setColor(SK_ColorDKGRAY);
    p.setShader(fOnShader);
    canvas->drawRect(r, p);

    p.setColor(SK_ColorWHITE);
    p.setShader(fOffShader);
    r.fLeft = r.fRight;
    r.fRight = this->width() - SK_Scalar1;
    if (r.width() > 0)
        canvas->drawRect(r, p);
}

#include "include/core/SkImageDecoder.h"

static SkShader* inflate_shader(const char file[])
{
    SkBitmap    bm;

    return SkImageDecoder::DecodeFile(file, &bm) ?
            SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode) :
            nullptr;
}

void SkProgressView::onInflate(const SkDOM& dom, const SkDOM::Node* node)
{
    this->INHERITED::onInflate(dom, node);

    const char* s;

    SkASSERT(fOnShader == nullptr);
    SkASSERT(fOffShader == nullptr);

    if ((s = dom.findAttr(node, "src-on")) != nullptr)
        fOnShader = inflate_shader(s);
    if ((s = dom.findAttr(node, "src-off")) != nullptr)
        fOffShader = inflate_shader(s);
    (void)dom.findBool(node, "do-interp", &fDoInterp);
}

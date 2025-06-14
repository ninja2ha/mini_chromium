/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLayerDrawLooper_DEFINED
#define SkLayerDrawLooper_DEFINED

#include "include/core/SkDrawLooper.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkXfermode.h"

class SK_API SkLayerDrawLooper : public SkDrawLooper {
public:
    virtual ~SkLayerDrawLooper();

    /**
     *  Bits specifies which aspects of the layer's paint should replace the
     *  corresponding aspects on the draw's paint.
     *  kEntirePaint_Bits means use the layer's paint completely.
     *  0 means ignore the layer's paint... except for fColorMode, which is
     *  always applied.
     */
    enum Bits {
        kStyle_Bit      = 1 << 0,   //!< use this layer's Style/stroke settings
        kTextSkewX_Bit  = 1 << 1,   //!< use this layer's textskewx
        kPathEffect_Bit = 1 << 2,   //!< use this layer's patheffect
        kMaskFilter_Bit = 1 << 3,   //!< use this layer's maskfilter
        kShader_Bit     = 1 << 4,   //!< use this layer's shader
        kColorFilter_Bit = 1 << 5,  //!< use this layer's colorfilter
        kXfermode_Bit   = 1 << 6,   //!< use this layer's xfermode

        /**
         *  Use the layer's paint entirely, with these exceptions:
         *  - We never override the draw's paint's text_encoding, since that is
         *    used to interpret the text/len parameters in draw[Pos]Text.
         *  - Color is always computed using the LayerInfo's fColorMode.
         */
        kEntirePaint_Bits = -1

    };
    typedef int32_t BitFlags;

    /**
     *  Info for how to apply the layer's paint and offset.
     *
     *  fColorMode controls how we compute the final color for the layer:
     *      The layer's paint's color is treated as the SRC
     *      The draw's paint's color is treated as the DST
     *      final-color = Mode(layers-color, draws-color);
     *  Any SkXfermode::Mode will work. Two common choices are:
     *      kSrc_Mode: to use the layer's color, ignoring the draw's
     *      kDst_Mode: to just keep the draw's color, ignoring the layer's
     */
    struct SK_API LayerInfo {
        BitFlags            fPaintBits;
        SkXfermode::Mode    fColorMode;
        SkVector            fOffset;
        bool                fPostTranslate; //!< applies to fOffset

        /**
         *  Initial the LayerInfo. Defaults to settings that will draw the
         *  layer with no changes: e.g.
         *      fPaintBits == 0
         *      fColorMode == kDst_Mode
         *      fOffset == (0, 0)
         */
        LayerInfo();
    };

    SkDrawLooper::Context* createContext(SkCanvas*, void* storage) const override;

    size_t contextSize() const override { return sizeof(LayerDrawLooperContext); }

    bool asABlurShadow(BlurShadowRec* rec) const override;

    SK_TO_STRING_OVERRIDE()

    Factory getFactory() const override { return CreateProc; }
    static SkFlattenable* CreateProc(SkReadBuffer& buffer);

protected:
    SkLayerDrawLooper();

    void flatten(SkWriteBuffer&) const override;

private:
    struct Rec {
        Rec*    fNext;
        SkPaint fPaint;
        LayerInfo fInfo;
    };
    Rec*    fRecs;
    int     fCount;

    // state-machine during the init/next cycle
    class LayerDrawLooperContext : public SkDrawLooper::Context {
    public:
        explicit LayerDrawLooperContext(const SkLayerDrawLooper* looper);

    protected:
        bool next(SkCanvas*, SkPaint* paint) override;

    private:
        Rec* fCurrRec;

        static void ApplyInfo(SkPaint* dst, const SkPaint& src, const LayerInfo&);
    };

    typedef SkDrawLooper INHERITED;

public:
    class SK_API Builder {
    public:
        Builder();
        ~Builder();

        /**
         *  Call for each layer you want to add (from top to bottom).
         *  This returns a paint you can modify, but that ptr is only valid until
         *  the next call made to addLayer().
         */
        SkPaint* addLayer(const LayerInfo&);

        /**
         *  This layer will draw with the original paint, at the specified offset
         */
        void addLayer(SkScalar dx, SkScalar dy);

        /**
         *  This layer will with the original paint and no offset.
         */
        void addLayer() { this->addLayer(0, 0); }

        /// Similar to addLayer, but adds a layer to the top.
        SkPaint* addLayerOnTop(const LayerInfo&);

        /**
          * Pass list of layers on to newly built looper and return it. This will
          * also reset the builder, so it can be used to build another looper.
          */
        SkLayerDrawLooper* detachLooper();

    private:
        Rec* fRecs;
        Rec* fTopRec;
        int  fCount;
    };
};

#endif

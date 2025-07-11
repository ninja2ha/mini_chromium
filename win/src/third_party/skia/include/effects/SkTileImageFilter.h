/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileImageFilter_DEFINED
#define SkTileImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

class SK_API SkTileImageFilter : public SkImageFilter {
    typedef SkImageFilter INHERITED;

public:
    /** Create a tile image filter
        @param src  Defines the pixels to tile
        @param dst  Defines the pixels where tiles are drawn
        @param input    Input from which the subregion defined by srcRect will be tiled
    */
    static SkImageFilter* Create(const SkRect& src, const SkRect& dst, SkImageFilter* input);

    bool onFilterImage(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                       SkBitmap* dst, SkIPoint* offset) const override;
    bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                        SkIRect* dst) const override;
    void onFilterNodeBounds(const SkIRect&, const SkMatrix&, SkIRect*, MapDirection) const override;
    void computeFastBounds(const SkRect& src, SkRect* dst) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTileImageFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    SkTileImageFilter(const SkRect& srcRect, const SkRect& dstRect, SkImageFilter* input)
        : INHERITED(1, &input, NULL), fSrcRect(srcRect), fDstRect(dstRect) {}

    SkRect fSrcRect;
    SkRect fDstRect;
};

#endif

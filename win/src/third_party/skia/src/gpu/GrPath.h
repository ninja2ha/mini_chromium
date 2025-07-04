/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPath_DEFINED
#define GrPath_DEFINED

#include "include/gpu/GrGpuResource.h"
#include "src/gpu/GrStrokeInfo.h"
#include "src/gpu/GrPathRendering.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"

class GrPath : public GrGpuResource {
public:
    /**
     * Initialize to a path with a fixed stroke. Stroke must not be hairline.
     */
    GrPath(GrGpu* gpu, const SkPath& skPath, const GrStrokeInfo& stroke)
        : INHERITED(gpu, kCached_LifeCycle)
        , fBounds(SkRect::MakeEmpty())
        , fFillType(GrPathRendering::kWinding_FillType)
#ifdef SK_DEBUG
        , fSkPath(skPath)
        , fStroke(stroke)
#endif
    {
    }

    static void ComputeKey(const SkPath& path, const GrStrokeInfo& stroke, GrUniqueKey* key,
                           bool* outIsVolatile);

    const SkRect& getBounds() const { return fBounds; }

    GrPathRendering::FillType getFillType() const { return fFillType; }
#ifdef SK_DEBUG
    bool isEqualTo(const SkPath& path, const GrStrokeInfo& stroke) const;
#endif

protected:
    // Subclass should init these.
    SkRect fBounds;
    GrPathRendering::FillType fFillType;
#ifdef SK_DEBUG
    SkPath fSkPath;
    GrStrokeInfo fStroke;
#endif

private:
    typedef GrGpuResource INHERITED;
};

#endif

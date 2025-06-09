/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectBatchFactory_DEFINED
#define GrRectBatchFactory_DEFINED

#include "src/gpu/batches/GrAAFillRectBatch.h"
#include "src/gpu/batches/GrAAStrokeRectBatch.h"
#include "include/gpu/GrColor.h"
#include "src/gpu/batches/GrNonAAFillRectBatch.h"
#include "src/gpu/batches/GrNonAAStrokeRectBatch.h"
#include "include/core/SkMatrix.h"

class GrBatch;
struct SkRect;
class SkStrokeRec;

/*
 * A factory for returning batches which can draw rectangles.
 */
namespace GrRectBatchFactory {

inline GrDrawBatch* CreateNonAAFill(GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect* localRect,
                                    const SkMatrix* localMatrix) {
    if (viewMatrix.hasPerspective() || (localMatrix && localMatrix->hasPerspective())) {
        return GrNonAAFillRectBatch::CreateWithPerspective(color, viewMatrix, rect, localRect,
                                                           localMatrix);
    } else {
        return GrNonAAFillRectBatch::Create(color, viewMatrix, rect, localRect, localMatrix);
    }
}

inline GrDrawBatch* CreateAAFill(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkRect& devRect) {
    return GrAAFillRectBatch::Create(color, viewMatrix, rect, devRect);
}

inline GrDrawBatch* CreateAAFill(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix,
                                 const SkRect& rect,
                                 const SkRect& devRect) {
    return GrAAFillRectBatch::Create(color, viewMatrix, localMatrix, rect, devRect);
}

inline GrDrawBatch* CreateNonAAStroke(GrColor color,
                                      const SkMatrix& viewMatrix,
                                      const SkRect& rect,
                                      SkScalar strokeWidth,
                                      bool snapToPixelCenters) {
    return GrNonAAStrokeRectBatch::Create(color, viewMatrix, rect, strokeWidth, snapToPixelCenters);
}

inline GrDrawBatch* CreateAAStroke(GrColor color,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkStrokeRec& stroke) {
    return GrAAStrokeRectBatch::Create(color, viewMatrix, rect, stroke);
}

// First rect is outer; second rect is inner
GrDrawBatch* CreateAAFillNestedRects(GrColor,
                                     const SkMatrix& viewMatrix,
                                     const SkRect rects[2]);

};

#endif

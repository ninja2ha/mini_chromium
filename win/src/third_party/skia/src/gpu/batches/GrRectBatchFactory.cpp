/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/batches/GrRectBatchFactory.h"

#include "src/gpu/batches/GrAAStrokeRectBatch.h"

#include "include/core/SkStrokeRec.h"

namespace GrRectBatchFactory {

GrDrawBatch* CreateAAFillNestedRects(GrColor color,
                                     const SkMatrix& viewMatrix,
                                     const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[0].isEmpty() && !rects[1].isEmpty());

    SkRect devOutside, devInside;
    viewMatrix.mapRect(&devOutside, rects[0]);
    viewMatrix.mapRect(&devInside, rects[1]);

    return GrAAStrokeRectBatch::Create(color, viewMatrix, devOutside, devOutside, devInside, true,
                                       devInside.isEmpty());
}

};

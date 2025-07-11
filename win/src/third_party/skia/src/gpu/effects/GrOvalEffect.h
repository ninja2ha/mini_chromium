/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalEffect_DEFINED
#define GrOvalEffect_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/GrTypesPriv.h"

class GrFragmentProcessor;
struct SkRect;

namespace GrOvalEffect {
    /**
     * Creates an effect that performs clipping against an oval.
     */
    GrFragmentProcessor* Create(GrPrimitiveEdgeType, const SkRect&);
};

#endif

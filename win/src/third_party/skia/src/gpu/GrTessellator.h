/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellator_DEFINED
#define GrTessellator_DEFINED

#include "include/core/SkPath.h"
#include "src/gpu/GrResourceProvider.h"

/**
 * Provides utility functions for converting paths to a collection of triangles.
 */

#define TESSELLATOR_WIREFRAME 0

namespace GrTessellator {

struct WindingVertex {
    SkPoint fPos;
    int fWinding;
};

// Triangulates a path to an array of vertices. Each triangle is represented as a set of three
// WindingVertex entries, each of which contains the position and winding count (which is the same
// for all three vertices of a triangle). The 'verts' out parameter is set to point to the resultant
// vertex array. CALLER IS RESPONSIBLE for deleting this buffer to avoid a memory leak!
int PathToVertices(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds, 
                   WindingVertex** verts);

int PathToTriangles(const SkPath& path, SkScalar tolerance, const SkRect& clipBounds, 
                    GrResourceProvider* resourceProvider, 
                    SkAutoTUnref<GrVertexBuffer>& vertexBuffer, bool canMapVB, bool* isLinear);

}

#endif

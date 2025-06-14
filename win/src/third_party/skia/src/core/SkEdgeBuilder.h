
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkEdgeBuilder_DEFINED
#define SkEdgeBuilder_DEFINED

#include "include/core/SkChunkAlloc.h"
#include "include/core/SkRect.h"
#include "include/core/SkTDArray.h"

struct SkEdge;
class SkEdgeClipper;
class SkPath;

class SkEdgeBuilder {
public:
    SkEdgeBuilder();

    // returns the number of built edges. The array of those edge pointers
    // is returned from edgeList().
    int build(const SkPath& path, const SkIRect* clip, int shiftUp, bool clipToTheRight);

    SkEdge** edgeList() { return fEdgeList; }

private:
    SkChunkAlloc        fAlloc;
    SkTDArray<SkEdge*>  fList;

    /*
     *  If we're in general mode, we allcoate the pointers in fList, and this
     *  will point at fList.begin(). If we're in polygon mode, fList will be
     *  empty, as we will have preallocated room for the pointers in fAlloc's
     *  block, and fEdgeList will point into that.
     */
    SkEdge**    fEdgeList;

    int         fShiftUp;

public:
    void addLine(const SkPoint pts[]);
    void addQuad(const SkPoint pts[]);
    void addCubic(const SkPoint pts[]);
    void addClipper(SkEdgeClipper*);

    int buildPoly(const SkPath& path, const SkIRect* clip, int shiftUp, bool clipToTheRight);
};

#endif


/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMeshUtils_DEFINED
#define SkMeshUtils_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkColor.h"

class SkBitmap;
class SkCanvas;
class SkPaint;

class SkMeshIndices {
public:
    SkMeshIndices();
    ~SkMeshIndices();

    bool init(int texW, int texH, int rows, int cols) {
        return this->init(NULL, NULL, texW, texH, rows, cols);
    }

    bool init(SkPoint tex[], uint16_t indices[],
              int texW, int texH, int rows, int cols);

    int             indexCount() const { return fIndexCount; }
    const uint16_t* indices() const { return fIndices; }

    size_t          texCount() const { return fTexCount; }
    const SkPoint*  tex() const { return fTex; }

private:
    int         fIndexCount, fTexCount;
    SkPoint*    fTex;
    uint16_t*   fIndices;
    void*       fStorage; // may be null
};

class SkMeshUtils {
public:
    static void Draw(SkCanvas*, const SkBitmap&, int rows, int cols,
                     const SkPoint verts[], const SkColor colors[],
                     const SkPaint& paint);
};

#endif


/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPath_DEFINED
#define GrGLPath_DEFINED

#include "src/gpu/GrPath.h"
#include "include/gpu/gl/GrGLTypes.h"

class GrGLGpu;

/**
 * Currently this represents a path built using GL_NV_path_rendering. If we
 * support other GL path extensions then this would have to have a type enum
 * and/or be subclassed.
 */

class GrGLPath : public GrPath {
public:
    static bool InitPathObjectPathDataCheckingDegenerates(GrGLGpu*,
                                                          GrGLuint pathID,
                                                          const SkPath&);
    static void InitPathObjectPathData(GrGLGpu*,
                                       GrGLuint pathID,
                                       const SkPath&);
    static void InitPathObjectStroke(GrGLGpu* gpu, GrGLuint pathID, const GrStrokeInfo& stroke);

    static void InitPathObjectEmptyPath(GrGLGpu*, GrGLuint pathID);


    GrGLPath(GrGLGpu* gpu, const SkPath& path, const GrStrokeInfo& stroke);
    GrGLuint pathID() const { return fPathID; }

    bool shouldStroke() const { return fShouldStroke; }
    bool shouldFill() const { return fShouldFill; }
protected:
    void onRelease() override;
    void onAbandon() override;

private:
    // TODO: Figure out how to get an approximate size of the path in Gpu memory.
    size_t onGpuMemorySize() const override { return 100; }

    GrGLuint fPathID;
    bool fShouldStroke;
    bool fShouldFill;

    typedef GrPath INHERITED;
};

#endif

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramDataManager_DEFINED
#define GrGLProgramDataManager_DEFINED

#include "src/gpu/glsl/GrGLSLProgramDataManager.h"

#include "src/gpu/GrAllocator.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/glsl/GrGLSLShaderVar.h"

#include "include/core/SkTArray.h"

class GrGLGpu;
class SkMatrix;
class GrGLProgram;

/** Manages the resources used by a shader program.
 * The resources are objects the program uses to communicate with the
 * application code.
 */
class GrGLProgramDataManager : public GrGLSLProgramDataManager {
public:
    struct UniformInfo {
        GrGLSLShaderVar fVariable;
        uint32_t        fVisibility;
        GrGLint         fLocation;
    };

    struct VaryingInfo {
        GrGLSLShaderVar fVariable;
        GrGLint         fLocation;
    };

    // This uses an allocator rather than array so that the GrGLSLShaderVars don't move in memory
    // after they are inserted. Users of GrGLShaderBuilder get refs to the vars and ptrs to their
    // name strings. Otherwise, we'd have to hand out copies.
    typedef GrTAllocator<UniformInfo> UniformInfoArray;
    typedef GrTAllocator<VaryingInfo> VaryingInfoArray;

    GrGLProgramDataManager(GrGLGpu*, GrGLuint programID, const UniformInfoArray&,
                           const VaryingInfoArray&);

    /** Functions for uploading uniform values. The varities ending in v can be used to upload to an
     *  array of uniforms. arrayCount must be <= the array count of the uniform.
     */
    void setSampler(UniformHandle, int texUnit) const;

    void set1f(UniformHandle, float v0) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2f(UniformHandle, float, float) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set3f(UniformHandle, float, float, float) const override;
    void set3fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set4f(UniformHandle, float, float, float, float) const override;
    void set4fv(UniformHandle, int arrayCount, const float v[]) const override;
    // matrices are column-major, the first three upload a single matrix, the latter three upload
    // arrayCount matrices into a uniform array.
    void setMatrix3f(UniformHandle, const float matrix[]) const override;
    void setMatrix4f(UniformHandle, const float matrix[]) const override;
    void setMatrix3fv(UniformHandle, int arrayCount, const float matrices[]) const override;
    void setMatrix4fv(UniformHandle, int arrayCount, const float matrices[]) const override;

    // convenience method for uploading a SkMatrix to a 3x3 matrix uniform
    void setSkMatrix(UniformHandle, const SkMatrix&) const override;

    // for nvpr only
    void setPathFragmentInputTransform(VaryingHandle u, int components,
                                       const SkMatrix& matrix) const override;

private:
    enum {
        kUnusedUniform = -1,
    };

    struct Uniform {
        GrGLint     fVSLocation;
        GrGLint     fFSLocation;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    enum {
        kUnusedPathProcVarying = -1,
    };
    struct PathProcVarying {
        GrGLint     fLocation;
        SkDEBUGCODE(
            GrSLType    fType;
            int         fArrayCount;
        );
    };

    SkDEBUGCODE(void printUnused(const Uniform&) const;)

    SkTArray<Uniform, true> fUniforms;
    SkTArray<PathProcVarying, true> fPathProcVaryings;
    GrGLGpu* fGpu;
    GrGLuint fProgramID;

    typedef GrGLSLProgramDataManager INHERITED;
};

#endif

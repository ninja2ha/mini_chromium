/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVertexArray_DEFINED
#define GrGLVertexArray_DEFINED

#include "include/gpu/GrTypesPriv.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/core/SkTArray.h"

class GrGLVertexBuffer;
class GrGLIndexBuffer;
class GrGLGpu;

struct GrGLAttribLayout {
    GrGLint     fCount;
    GrGLenum    fType;
    GrGLboolean fNormalized;
};

static inline const GrGLAttribLayout& GrGLAttribTypeToLayout(GrVertexAttribType type) {
    SkASSERT(type >= 0 && type < kGrVertexAttribTypeCount);
    static const GrGLAttribLayout kLayouts[kGrVertexAttribTypeCount] = {
        {1, GR_GL_FLOAT, false},         // kFloat_GrVertexAttribType
        {2, GR_GL_FLOAT, false},         // kVec2f_GrVertexAttribType
        {3, GR_GL_FLOAT, false},         // kVec3f_GrVertexAttribType
        {4, GR_GL_FLOAT, false},         // kVec4f_GrVertexAttribType
        {1, GR_GL_UNSIGNED_BYTE, true},  // kUByte_GrVertexAttribType
        {4, GR_GL_UNSIGNED_BYTE, true},  // kVec4ub_GrVertexAttribType
        {2, GR_GL_SHORT, false},         // kVec2s_GrVertexAttribType
    };
    GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
    GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
    GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
    GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
    GR_STATIC_ASSERT(4 == kUByte_GrVertexAttribType);
    GR_STATIC_ASSERT(5 == kVec4ub_GrVertexAttribType);
    GR_STATIC_ASSERT(6 == kVec2s_GrVertexAttribType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kLayouts) == kGrVertexAttribTypeCount);
    return kLayouts[type];
}

/**
 * This sets and tracks the vertex attribute array state. It is used internally by GrGLVertexArray
 * (below) but is separate because it is also used to track the state of vertex array object 0.
 */
class GrGLAttribArrayState {
public:
    explicit GrGLAttribArrayState(int arrayCount = 0) {
        this->resize(arrayCount);
    }

    void resize(int newCount) {
        fAttribArrayStates.resize_back(newCount);
        for (int i = 0; i < newCount; ++i) {
            fAttribArrayStates[i].invalidate();
        }
    }

    /**
     * This function enables and sets vertex attrib state for the specified attrib index. It is
     * assumed that the GrGLAttribArrayState is tracking the state of the currently bound vertex
     * array object.
     */
    void set(GrGLGpu*,
             int attribIndex,
             GrGLuint vertexBufferID,
             GrGLint size,
             GrGLenum type,
             GrGLboolean normalized,
             GrGLsizei stride,
             GrGLvoid* offset);

    /**
     * This function disables vertex attribs not present in the mask. It is assumed that the
     * GrGLAttribArrayState is tracking the state of the currently bound vertex array object.
     */
    void disableUnusedArrays(const GrGLGpu*, uint64_t usedAttribArrayMask);

    void invalidate() {
        int count = fAttribArrayStates.count();
        for (int i = 0; i < count; ++i) {
            fAttribArrayStates[i].invalidate();
        }
    }

    void notifyVertexBufferDelete(GrGLuint id) {
        int count = fAttribArrayStates.count();
        for (int i = 0; i < count; ++i) {
            if (fAttribArrayStates[i].fAttribPointerIsValid &&
                id == fAttribArrayStates[i].fVertexBufferID) {
                fAttribArrayStates[i].invalidate();
            }
        }
    }

    /**
     * The number of attrib arrays that this object is configured to track.
     */
    int count() const { return fAttribArrayStates.count(); }

private:
    /**
     * Tracks the state of glVertexAttribArray for an attribute index.
     */
    struct AttribArrayState {
            void invalidate() {
                fEnableIsValid = false;
                fAttribPointerIsValid = false;
            }

            bool        fEnableIsValid;
            bool        fAttribPointerIsValid;
            bool        fEnabled;
            GrGLuint    fVertexBufferID;
            GrGLint     fSize;
            GrGLenum    fType;
            GrGLboolean fNormalized;
            GrGLsizei   fStride;
            GrGLvoid*   fOffset;
    };

    SkSTArray<16, AttribArrayState, true> fAttribArrayStates;
};

/**
 * This class represents an OpenGL vertex array object. It manages the lifetime of the vertex array
 * and is used to track the state of the vertex array to avoid redundant GL calls.
 */
class GrGLVertexArray {
public:
    GrGLVertexArray(GrGLint id, int attribCount);

    /**
     * Binds this vertex array. If the ID has been deleted or abandoned then nullptr is returned.
     * Otherwise, the GrGLAttribArrayState that is tracking this vertex array's attrib bindings is
     * returned.
     */
    GrGLAttribArrayState* bind(GrGLGpu*);

    /**
     * This is a version of the above function that also binds an index buffer to the vertex
     * array object.
     */
    GrGLAttribArrayState* bindWithIndexBuffer(GrGLGpu* gpu, GrGLuint indexBufferID);

    void notifyIndexBufferDelete(GrGLuint bufferID);

    void notifyVertexBufferDelete(GrGLuint id) {
        fAttribArrays.notifyVertexBufferDelete(id);
    }

    GrGLuint arrayID() const { return fID; }

    void invalidateCachedState();

private:
    GrGLuint                fID;
    GrGLAttribArrayState    fAttribArrays;
    GrGLuint                fIndexBufferID;
    bool                    fIndexBufferIDIsValid;
};

#endif

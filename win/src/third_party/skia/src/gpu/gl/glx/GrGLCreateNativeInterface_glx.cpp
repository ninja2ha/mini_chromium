/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLAssembleInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

#include <GL/glx.h>

static GrGLFuncPtr glx_get(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    SkASSERT(glXGetCurrentContext());
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(name));
}

const GrGLInterface* GrGLCreateNativeInterface() {
    if (nullptr == glXGetCurrentContext()) {
        return nullptr;
    }

    return GrGLAssembleInterface(nullptr, glx_get);
}

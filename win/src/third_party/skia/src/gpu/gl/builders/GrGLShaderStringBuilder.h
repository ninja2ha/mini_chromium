/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderStringBuilder_DEFINED
#define GrGLShaderStringBuilder_DEFINED

#include "src/gpu/GrAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/gl/GrGLContext.h"
#include "include/core/SkTypes.h"

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const char** strings,
                                    int* lengths,
                                    int count,
                                    GrGpu::Stats*);

#endif

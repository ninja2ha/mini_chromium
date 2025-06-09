/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLInitGLSL_DEFINED
#define GrGLInitGLSL_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/glsl/GrGLSL.h"
#include "include/gpu/GrColor.h"
#include "include/gpu/GrTypesPriv.h"
#include "include/core/SkString.h"

class GrGLContextInfo;

/**
 * Gets the most recent GLSL Generation compatible with the OpenGL context.
 */
bool GrGLGetGLSLGeneration(const GrGLInterface* gl, GrGLSLGeneration* generation);


#endif


/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShaderObj_DEFINED
#define GrShaderObj_DEFINED

#include "src/gpu/gl/debug/GrFakeRefObj.h"
#include "src/gpu/gl/GrGLDefines.h"

////////////////////////////////////////////////////////////////////////////////
class GrShaderObj : public GrFakeRefObj {
    GR_DEFINE_CREATOR(GrShaderObj);

public:
    GrShaderObj()
        : GrFakeRefObj()
        , fType(GR_GL_VERTEX_SHADER)    {}

    void setType(GrGLenum type)         { fType = type; }
    GrGLenum getType()                  { return fType; }

    void deleteAction() override;

protected:
private:
    GrGLenum fType;  // either GR_GL_VERTEX_SHADER or GR_GL_FRAGMENT_SHADER

    typedef GrFakeRefObj INHERITED;
};

#endif // GrShaderObj_DEFINED

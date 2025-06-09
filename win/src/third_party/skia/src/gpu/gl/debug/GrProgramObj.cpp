
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/debug/GrProgramObj.h"
#include "src/gpu/gl/debug/GrShaderObj.h"

void GrProgramObj::AttachShader(GrShaderObj *shader) {
    shader->ref();
    fShaders.push_back(shader);
}

void GrProgramObj::deleteAction() {

    // shaders are automatically detached from a deleted program. They will only be
    // deleted if they were marked for deletion by a prior call to glDeleteShader
    for (int i = 0; i < fShaders.count(); ++i) {
        fShaders[i]->unref();
    }
    fShaders.reset();

    this->INHERITED::deleteAction();
}

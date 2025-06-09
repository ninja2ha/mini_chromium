/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLProgram.h"

#include "src/gpu/GrAllocator.h"
#include "include/gpu/GrProcessor.h"
#include "include/gpu/GrCoordTransform.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLPathRendering.h"
#include "src/gpu/GrPathProcessor.h"
#include "src/gpu/GrPipeline.h"
#include "include/gpu/GrXferProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"
#include "include/core/SkXfermode.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLProgram::GrGLProgram(GrGLGpu* gpu,
                         const GrProgramDesc& desc,
                         const BuiltinUniformHandles& builtinUniforms,
                         GrGLuint programID,
                         const UniformInfoArray& uniforms,
                         const VaryingInfoArray& pathProcVaryings,
                         GrGLSLPrimitiveProcessor* geometryProcessor,
                         GrGLSLXferProcessor* xferProcessor,
                         const GrGLSLFragProcs& fragmentProcessors,
                         SkTArray<UniformHandle>* passSamplerUniforms)
    : fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(fragmentProcessors)
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, programID, uniforms, pathProcVaryings) {
    fSamplerUniforms.swap(passSamplerUniforms);
    // Assign texture units to sampler uniforms one time up front.
    GL_CALL(UseProgram(fProgramID));
    for (int i = 0; i < fSamplerUniforms.count(); i++) {
        fProgramDataManager.setSampler(fSamplerUniforms[i], i);
    }
}

GrGLProgram::~GrGLProgram() {
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        delete fFragmentProcessors[i];
    }
}

void GrGLProgram::abandon() {
    fProgramID = 0;
}

///////////////////////////////////////////////////////////////////////////////

static void append_texture_bindings(const GrProcessor& processor,
                                    SkTArray<const GrTextureAccess*>* textureBindings) {
    if (int numTextures = processor.numTextures()) {
        const GrTextureAccess** bindings = textureBindings->push_back_n(numTextures);
        int i = 0;
        do {
            bindings[i] = &processor.textureAccess(i);
        } while (++i < numTextures);
    }
}

void GrGLProgram::setData(const GrPrimitiveProcessor& primProc,
                          const GrPipeline& pipeline,
                          SkTArray<const GrTextureAccess*>* textureBindings) {
    this->setRenderTargetState(primProc, pipeline);

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms
    fGeometryProcessor->setData(fProgramDataManager, primProc);
    append_texture_bindings(primProc, textureBindings);

    this->setFragmentData(primProc, pipeline, textureBindings);

    const GrXferProcessor& xp = pipeline.getXferProcessor();
    fXferProcessor->setData(fProgramDataManager, xp);
    append_texture_bindings(xp, textureBindings);
}

void GrGLProgram::setFragmentData(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline,
                                  SkTArray<const GrTextureAccess*>* textureBindings) {
    int numProcessors = fFragmentProcessors.count();
    for (int i = 0; i < numProcessors; ++i) {
        const GrFragmentProcessor& processor = pipeline.getFragmentProcessor(i);
        fFragmentProcessors[i]->setData(fProgramDataManager, processor);
        this->setTransformData(primProc, processor, i);
        append_texture_bindings(processor, textureBindings);
    }
}
void GrGLProgram::setTransformData(const GrPrimitiveProcessor& primProc,
                                   const GrFragmentProcessor& processor,
                                   int index) {
    fGeometryProcessor->setTransformData(primProc, fProgramDataManager, index,
                                         processor.coordTransforms());
}

void GrGLProgram::setRenderTargetState(const GrPrimitiveProcessor& primProc,
                                       const GrPipeline& pipeline) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != pipeline.getRenderTarget()->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni,
                                   SkIntToScalar(pipeline.getRenderTarget()->height()));
    }

    // set RT adjustment
    const GrRenderTarget* rt = pipeline.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    if (!primProc.isPathRendering()) {
        if (fRenderTargetState.fRenderTargetOrigin != rt->origin() ||
            fRenderTargetState.fRenderTargetSize != size) {
            fRenderTargetState.fRenderTargetSize = size;
            fRenderTargetState.fRenderTargetOrigin = rt->origin();

            float rtAdjustmentVec[4];
            fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
            fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
        }
    } else {
        SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());
        const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
        fGpu->glPathRendering()->setProjectionMatrix(pathProc.viewMatrix(),
                                                     size, rt->origin());
    }
}

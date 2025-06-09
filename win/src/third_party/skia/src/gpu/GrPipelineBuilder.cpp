/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrPipelineBuilder.h"

#include "include/gpu/GrBlend.h"
#include "include/gpu/GrPaint.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcOptInfo.h"
#include "include/gpu/GrXferProcessor.h"
#include "src/gpu/batches/GrBatch.h"
#include "include/gpu/effects/GrPorterDuffXferProcessor.h"

GrPipelineBuilder::GrPipelineBuilder()
    : fFlags(0x0), fDrawFace(kBoth_DrawFace) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
}

GrPipelineBuilder::GrPipelineBuilder(const GrPaint& paint, GrRenderTarget* rt, const GrClip& clip) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)

    for (int i = 0; i < paint.numColorFragmentProcessors(); ++i) {
        fColorFragmentProcessors.push_back(SkRef(paint.getColorFragmentProcessor(i)));
    }

    for (int i = 0; i < paint.numCoverageFragmentProcessors(); ++i) {
        fCoverageFragmentProcessors.push_back(SkRef(paint.getCoverageFragmentProcessor(i)));
    }

    fXPFactory.reset(SkSafeRef(paint.getXPFactory()));

    this->setRenderTarget(rt);

    // These have no equivalent in GrPaint, set them to defaults
    fDrawFace = kBoth_DrawFace;
    fStencilSettings.setDisabled();
    fFlags = 0;

    fClip = clip;

    this->setState(GrPipelineBuilder::kHWAntialias_Flag,
                   rt->isUnifiedMultisampled() && paint.isAntiAlias());
}

//////////////////////////////////////////////////////////////////////////////s

bool GrPipelineBuilder::willXPNeedDstTexture(const GrCaps& caps,
                                             const GrPipelineOptimizations& optimizations) const {
    if (this->getXPFactory()) {
        return this->getXPFactory()->willNeedDstTexture(caps, optimizations, 
                                                        this->hasMixedSamples());
    }
    return GrPorterDuffXPFactory::SrcOverWillNeedDstTexture(caps, optimizations,
                                                            this->hasMixedSamples());
}

void GrPipelineBuilder::AutoRestoreFragmentProcessorState::set(
                                                         const GrPipelineBuilder* pipelineBuilder) {
    if (fPipelineBuilder) {
        int m = fPipelineBuilder->numColorFragmentProcessors() - fColorEffectCnt;
        SkASSERT(m >= 0);
        for (int i = 0; i < m; ++i) {
            fPipelineBuilder->fColorFragmentProcessors.fromBack(i)->unref();
        }
        fPipelineBuilder->fColorFragmentProcessors.pop_back_n(m);

        int n = fPipelineBuilder->numCoverageFragmentProcessors() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        for (int i = 0; i < n; ++i) {
            fPipelineBuilder->fCoverageFragmentProcessors.fromBack(i)->unref();
        }
        fPipelineBuilder->fCoverageFragmentProcessors.pop_back_n(n);
        SkDEBUGCODE(--fPipelineBuilder->fBlockEffectRemovalCnt;)
    }
    fPipelineBuilder = const_cast<GrPipelineBuilder*>(pipelineBuilder);
    if (nullptr != pipelineBuilder) {
        fColorEffectCnt = pipelineBuilder->numColorFragmentProcessors();
        fCoverageEffectCnt = pipelineBuilder->numCoverageFragmentProcessors();
        SkDEBUGCODE(++pipelineBuilder->fBlockEffectRemovalCnt;)
    }
}

////////////////////////////////////////////////////////////////////////////////

GrPipelineBuilder::~GrPipelineBuilder() {
    SkASSERT(0 == fBlockEffectRemovalCnt);
    for (int i = 0; i < fColorFragmentProcessors.count(); ++i) {
        fColorFragmentProcessors[i]->unref();
    }
    for (int i = 0; i < fCoverageFragmentProcessors.count(); ++i) {
        fCoverageFragmentProcessors[i]->unref();
    }
}

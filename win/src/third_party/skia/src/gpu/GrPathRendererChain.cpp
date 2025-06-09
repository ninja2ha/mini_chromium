
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/GrPathRendererChain.h"

#include "include/gpu/GrCaps.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrGpu.h"

#include "src/gpu/batches/GrAAConvexPathRenderer.h"
#include "src/gpu/batches/GrAADistanceFieldPathRenderer.h"
#include "src/gpu/batches/GrAAHairLinePathRenderer.h"
#include "src/gpu/batches/GrAALinearizingConvexPathRenderer.h"
#include "src/gpu/batches/GrDashLinePathRenderer.h"
#include "src/gpu/batches/GrDefaultPathRenderer.h"
#include "src/gpu/batches/GrStencilAndCoverPathRenderer.h"
#include "src/gpu/batches/GrTessellatingPathRenderer.h"

GrPathRendererChain::GrPathRendererChain(GrContext* context) {
    const GrCaps& caps = *context->caps();
    this->addPathRenderer(new GrDashLinePathRenderer)->unref();

    if (GrPathRenderer* pr = GrStencilAndCoverPathRenderer::Create(context->resourceProvider(),
                                                                   caps)) {
        this->addPathRenderer(pr)->unref();
    }
    this->addPathRenderer(new GrTessellatingPathRenderer)->unref();
    this->addPathRenderer(new GrAAHairLinePathRenderer)->unref();
    this->addPathRenderer(new GrAAConvexPathRenderer)->unref();
    this->addPathRenderer(new GrAALinearizingConvexPathRenderer)->unref();
    this->addPathRenderer(new GrAADistanceFieldPathRenderer)->unref();
    this->addPathRenderer(new GrDefaultPathRenderer(caps.twoSidedStencilSupport(),
                                                    caps.stencilWrapOpsSupport()))->unref();
}

GrPathRendererChain::~GrPathRendererChain() {
    for (int i = 0; i < fChain.count(); ++i) {
        fChain[i]->unref();
    }
}

GrPathRenderer* GrPathRendererChain::addPathRenderer(GrPathRenderer* pr) {
    fChain.push_back() = pr;
    pr->ref();
    return pr;
}

GrPathRenderer* GrPathRendererChain::getPathRenderer(const GrPathRenderer::CanDrawPathArgs& args,
                                                     DrawType drawType,
                                                     GrPathRenderer::StencilSupport* stencilSupport) {
    GR_STATIC_ASSERT(GrPathRenderer::kNoSupport_StencilSupport <
                     GrPathRenderer::kStencilOnly_StencilSupport);
    GR_STATIC_ASSERT(GrPathRenderer::kStencilOnly_StencilSupport <
                     GrPathRenderer::kNoRestriction_StencilSupport);
    GrPathRenderer::StencilSupport minStencilSupport;
    if (kStencilOnly_DrawType == drawType) {
        minStencilSupport = GrPathRenderer::kStencilOnly_StencilSupport;
    } else if (kStencilAndColor_DrawType == drawType ||
               kStencilAndColorAntiAlias_DrawType == drawType) {
        minStencilSupport = GrPathRenderer::kNoRestriction_StencilSupport;
    } else {
        minStencilSupport = GrPathRenderer::kNoSupport_StencilSupport;
    }

    for (int i = 0; i < fChain.count(); ++i) {
        if (fChain[i]->canDrawPath(args)) {
            if (GrPathRenderer::kNoSupport_StencilSupport != minStencilSupport) {
                GrPathRenderer::StencilSupport support =
                                        fChain[i]->getStencilSupport(*args.fPath, *args.fStroke);
                if (support < minStencilSupport) {
                    continue;
                } else if (stencilSupport) {
                    *stencilSupport = support;
                }
            }
            return fChain[i];
        }
    }
    return nullptr;
}

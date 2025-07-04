/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawPathBatch_DEFINED
#define GrDrawPathBatch_DEFINED

#include "src/gpu/GrBatchFlushState.h"
#include "src/gpu/batches/GrDrawBatch.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrPath.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrPathProcessor.h"

#include "src/core/SkTLList.h"

class GrDrawPathBatchBase : public GrDrawBatch {
public:
    void computePipelineOptimizations(GrInitInvariantOutput* color, 
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setKnownFourComponents(fColor);
        coverage->setKnownSingleComponent(0xff);
        overrides->fUsePLSDstRead = false;
    }

    GrPathRendering::FillType fillType() const { return fFillType; }

    void setStencilSettings(const GrStencilSettings& stencil) { fStencilSettings = stencil; }

protected:
    GrDrawPathBatchBase(uint32_t classID, const SkMatrix& viewMatrix, GrColor initialColor,
                        GrPathRendering::FillType fill)
        : INHERITED(classID)
        , fViewMatrix(viewMatrix)
        , fColor(initialColor)
        , fFillType(fill) {}

    const GrStencilSettings& stencilSettings() const { return fStencilSettings; }
    const GrXPOverridesForBatch& overrides() const { return fOverrides; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    GrColor color() const { return fColor; }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fColor);
        fOverrides = overrides;
    }

    SkMatrix                                                fViewMatrix;
    GrColor                                                 fColor;
    GrPathRendering::FillType                               fFillType;
    GrStencilSettings                                       fStencilSettings;
    GrXPOverridesForBatch                                   fOverrides;

    typedef GrDrawBatch INHERITED;
};

class GrDrawPathBatch final : public GrDrawPathBatchBase {
public:
    DEFINE_BATCH_CLASS_ID

    // This can't return a more abstract type because we install the stencil settings late :(
    static GrDrawPathBatchBase* Create(const SkMatrix& viewMatrix, GrColor color,
                                       GrPathRendering::FillType fill, const GrPath* path) {
        return new GrDrawPathBatch(viewMatrix, color, fill, path);
    }

    const char* name() const override { return "DrawPath"; }

    SkString dumpInfo() const override;

private:
    GrDrawPathBatch(const SkMatrix& viewMatrix, GrColor color, GrPathRendering::FillType fill,
                    const GrPath* path)
        : INHERITED(ClassID(), viewMatrix, color, fill)
        , fPath(path) {
        fBounds = path->getBounds();
        viewMatrix.mapRect(&fBounds);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override;

    GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;

    typedef GrDrawPathBatchBase INHERITED;
};

// Template this if we decide to support index types other than 16bit
class GrDrawPathRangeBatch final : public GrDrawPathBatchBase {
public:
    typedef GrPathRendering::PathTransformType TransformType;

    DEFINE_BATCH_CLASS_ID

    struct InstanceData : public SkNoncopyable {
    public:
        static InstanceData* Alloc(TransformType transformType, int reserveCnt) {
            int transformSize = GrPathRendering::PathTransformSize(transformType);
            uint8_t* ptr = (uint8_t*)sk_malloc_throw(Align32(sizeof(InstanceData)) +
                                                     Align32(reserveCnt * sizeof(uint16_t)) +
                                                     reserveCnt * transformSize * sizeof(float));
            InstanceData* instanceData = (InstanceData*)ptr;
            instanceData->fIndices = (uint16_t*)&ptr[Align32(sizeof(InstanceData))];
            instanceData->fTransformValues = (float*)&ptr[Align32(sizeof(InstanceData)) +
                                                          Align32(reserveCnt * sizeof(uint16_t))];
            instanceData->fTransformType = transformType;
            instanceData->fInstanceCount = 0;
            instanceData->fRefCnt = 1;
            SkDEBUGCODE(instanceData->fReserveCnt = reserveCnt;)
            return instanceData;
        }

        // Overload this method if we start using other transform types.
        void append(uint16_t index, float x, float y) {
            SkASSERT(GrPathRendering::kTranslate_PathTransformType == fTransformType);
            SkASSERT(fInstanceCount < fReserveCnt);
            fIndices[fInstanceCount] = index;
            fTransformValues[2 * fInstanceCount] = x;
            fTransformValues[2 * fInstanceCount + 1] = y;
            ++fInstanceCount;
        }

        TransformType transformType() const { return fTransformType; }
        int count() const { return fInstanceCount; }

        const uint16_t* indices() const { return fIndices; }
        uint16_t* indices() { return fIndices; }

        const float* transformValues() const { return fTransformValues; }
        float* transformValues() { return fTransformValues; }

        void ref() const { ++fRefCnt; }

        void unref() const {
            if (0 == --fRefCnt) {
                sk_free(const_cast<InstanceData*>(this));
            }
        }

    private:
        static int Align32(int sizeInBytes) { return (sizeInBytes + 3) & ~3; }

        InstanceData() {}
        ~InstanceData() {}

        uint16_t*       fIndices;
        float*          fTransformValues;
        TransformType   fTransformType;
        int             fInstanceCount;
        mutable int     fRefCnt;
        SkDEBUGCODE(int fReserveCnt;)
    };

    // This can't return a more abstract type because we install the stencil settings late :(
    static GrDrawPathBatchBase* Create(const SkMatrix& viewMatrix, SkScalar scale, SkScalar x,
                                       SkScalar y, GrColor color, GrPathRendering::FillType fill,
                                       GrPathRange* range, const InstanceData* instanceData,
                                       const SkRect& bounds) {
        return new GrDrawPathRangeBatch(viewMatrix, scale, x, y, color, fill, range, instanceData,
                                        bounds);
    }

    const char* name() const override { return "DrawPathRange"; }

    SkString dumpInfo() const override;

private:
    GrDrawPathRangeBatch(const SkMatrix& viewMatrix, SkScalar scale, SkScalar x, SkScalar y,
                         GrColor color, GrPathRendering::FillType fill, GrPathRange* range,
                         const InstanceData* instanceData, const SkRect& bounds);

    TransformType transformType() const { return fDraws.head()->fInstanceData->transformType(); }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override;

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override;

    struct Draw {
        void set(const InstanceData* instanceData, SkScalar x, SkScalar y) {
            fInstanceData.reset(SkRef(instanceData));
            fX = x;
            fY = y;
        }

        SkAutoTUnref<const InstanceData>    fInstanceData;
        SkScalar                            fX, fY;
    };

    typedef GrPendingIOResource<const GrPathRange, kRead_GrIOType> PendingPathRange;
    typedef SkTLList<Draw, 4> DrawList;

    PendingPathRange    fPathRange;
    DrawList            fDraws;
    int                 fTotalPathCount;
    SkScalar            fScale;

    typedef GrDrawPathBatchBase INHERITED;
};

#endif

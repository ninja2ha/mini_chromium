
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSweepGradient_DEFINED
#define SkSweepGradient_DEFINED

#include "src/effects/gradients/SkGradientShaderPriv.h"

class SkSweepGradient : public SkGradientShaderBase {
public:
    SkSweepGradient(SkScalar cx, SkScalar cy, const Descriptor&);

    size_t contextSize() const override;

    class SweepGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        SweepGradientContext(const SkSweepGradient& shader, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

    private:
        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix*,
                                                   SkFilterQuality) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSweepGradient)

protected:
    void flatten(SkWriteBuffer& buffer) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    const SkPoint fCenter;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif

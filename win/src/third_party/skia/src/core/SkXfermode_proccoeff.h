/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermode_proccoeff_DEFINED
#define SkXfermode_proccoeff_DEFINED

#include "include/core/SkXfermode.h"
#include "src/core/SkReadBuffer.h"
#include "include/core/SkWriteBuffer.h"

struct ProcCoeff {
    SkXfermodeProc      fProc;
    SkXfermode::Coeff   fSC;
    SkXfermode::Coeff   fDC;
};

#define CANNOT_USE_COEFF    SkXfermode::Coeff(-1)

class SK_API SkProcCoeffXfermode : public SkXfermode {
public:
    SkProcCoeffXfermode(const ProcCoeff& rec, Mode mode) {
        fMode = mode;
        fProc = rec.fProc;
        // these may be valid, or may be CANNOT_USE_COEFF
        fSrcCoeff = rec.fSC;
        fDstCoeff = rec.fDC;
    }

    void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                const SkAlpha aa[]) const override;
    void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                const SkAlpha aa[]) const override;
    void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                const SkAlpha aa[]) const override;

    bool asMode(Mode* mode) const override;

    bool supportsCoverageAsAlpha() const override;

    bool isOpaque(SkXfermode::SrcColorOpacity opacityType) const override;

#if SK_SUPPORT_GPU
    bool asFragmentProcessor(const GrFragmentProcessor**,
                             const GrFragmentProcessor*) const override;

    bool asXPFactory(GrXPFactory**) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcCoeffXfermode)

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    Mode getMode() const { return fMode; }

    SkXfermodeProc getProc() const { return fProc; }

private:
    SkXfermodeProc  fProc;
    Mode            fMode;
    Coeff           fSrcCoeff, fDstCoeff;

    friend class SkXfermode;

    typedef SkXfermode INHERITED;
};

#endif // #ifndef SkXfermode_proccoeff_DEFINED

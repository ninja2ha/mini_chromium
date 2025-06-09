/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBitmapProcShader.h"
#include "include/core/SkColorFilter.h"
#include "src/core/SkColorFilterShader.h"
#include "src/core/SkColorShader.h"
#include "include/core/SkComposeShader.h"
#include "src/core/SkEmptyShader.h"
#include "include/core/SkFlattenable.h"
#include "src/image/SkImageShader.h"
#include "src/core/SkLocalMatrixShader.h"
#include "include/private/SkOnce.h"
#include "include/core/SkPathEffect.h"
#include "src/core/SkPictureShader.h"
#include "src/core/SkMatrixImageFilter.h"
#include "include/core/SkXfermode.h"

/*
 *  Registers all of the required effects subclasses for picture deserialization.
 *
 *  Optional subclasses (e.g. Blur) should be registered in the ports/ version of this file,
 *  inside the InitEffects() method.
 */
void SkFlattenable::PrivateInitializer::InitCore() {
    // Shader
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapProcShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmptyShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLocalMatrixShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPictureShader)

    // PathEffect
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposePathEffect)

    // ImageFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixImageFilter)

    // ColorFilter
    SkColorFilter::InitializeFlattenables();

    // Xfermode
    SkXfermode::InitializeFlattenables();

    // Now initialize any optional/additional effects (implemented in src/ports)
    InitEffects();
};

SK_DECLARE_STATIC_ONCE(once);
void SkFlattenable::InitializeFlattenablesIfNeeded() {
    SkOnce(&once, SkFlattenable::PrivateInitializer::InitCore);
}

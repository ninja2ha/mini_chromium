/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/ports/SkFontMgr.h"

struct SkEmbeddedResource { const uint8_t* data; size_t size; };
struct SkEmbeddedResourceHeader { const SkEmbeddedResource* entries; int count; };
SkFontMgr* SkFontMgr_New_Custom_Embedded(const SkEmbeddedResourceHeader* header);

extern "C" const SkEmbeddedResourceHeader SK_EMBEDDED_FONTS;
SkFontMgr* SkFontMgr::Factory() {
    return SkFontMgr_New_Custom_Embedded(&SK_EMBEDDED_FONTS);
}

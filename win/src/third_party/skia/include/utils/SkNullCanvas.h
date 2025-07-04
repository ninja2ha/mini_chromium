/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNullCanvas_DEFINED
#define SkNullCanvas_DEFINED

#include "include/core/SkBitmap.h"

class SkCanvas;

/**
 * Creates a canvas that draws nothing. This is useful for performance testing.
 */
SK_API SkCanvas* SkCreateNullCanvas();

#endif

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkNullCanvas.h"

#include "include/core/SkCanvas.h"
#include "include/utils/SkNWayCanvas.h"


SkCanvas* SkCreateNullCanvas() {
    // An N-Way canvas forwards calls to N canvas's. When N == 0 it's
    // effectively a null canvas.
    return new SkNWayCanvas(0, 0);
}

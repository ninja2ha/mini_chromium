/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/svg/SkSVGCanvas.h"
#include "src/svg/SkSVGDevice.h"

SkCanvas* SkSVGCanvas::Create(const SkRect& bounds, SkXMLWriter* writer) {
    // TODO: pass full bounds to the device
    SkISize size = bounds.roundOut().size();
    SkAutoTUnref<SkBaseDevice> device(SkSVGDevice::Create(size, writer));

    return new SkCanvas(device);
}

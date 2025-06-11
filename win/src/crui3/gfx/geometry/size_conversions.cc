// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/size_conversions.h"

#include "crui/gfx/geometry/safe_integer_conversions.h"

namespace crui {
namespace gfx {

Size ToFlooredSize(const SizeF& size) {
  int w = ToFlooredInt(size.width());
  int h = ToFlooredInt(size.height());
  return Size(w, h);
}

Size ToCeiledSize(const SizeF& size) {
  int w = ToCeiledInt(size.width());
  int h = ToCeiledInt(size.height());
  return Size(w, h);
}

Size ToRoundedSize(const SizeF& size) {
  int w = ToRoundedInt(size.width());
  int h = ToRoundedInt(size.height());
  return Size(w, h);
}

}  // namespace gfx
}  // namespace crui


// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/point_f.h"

#include "crbase/strings/stringprintf.h"

namespace crui {
namespace gfx {

void PointF::SetToMin(const PointF& other) {
  x_ = x_ <= other.x_ ? x_ : other.x_;
  y_ = y_ <= other.y_ ? y_ : other.y_;
}

void PointF::SetToMax(const PointF& other) {
  x_ = x_ >= other.x_ ? x_ : other.x_;
  y_ = y_ >= other.y_ ? y_ : other.y_;
}

std::string PointF::ToString() const {
  return cr::StringPrintf("%f,%f", x(), y());
}

PointF ScalePoint(const PointF& p, float x_scale, float y_scale) {
  PointF scaled_p(p);
  scaled_p.Scale(x_scale, y_scale);
  return scaled_p;
}


}  // namespace gfx
}  // namespace crui

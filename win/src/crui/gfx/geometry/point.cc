// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/point.h"

#include "crbase/strings/stringprintf.h"
#include "crbase/build_platform.h"
#include "crui/gfx/geometry/point_conversions.h"
#include "crui/gfx/geometry/point_f.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#elif defined(MINI_CHROMIUM_OS_MACOSX)
#include <ApplicationServices/ApplicationServices.h>
#endif

namespace crui {
namespace gfx {

#if defined(MINI_CHROMIUM_OS_WIN)
Point::Point(DWORD point) {
  POINTS points = MAKEPOINTS(point);
  x_ = points.x;
  y_ = points.y;
}

Point::Point(const POINT& point) : x_(point.x), y_(point.y) {
}

Point& Point::operator=(const POINT& point) {
  x_ = point.x;
  y_ = point.y;
  return *this;
}
#elif defined(MINI_CHROMIUM_OS_MACOSX)
Point::Point(const CGPoint& point) : x_(point.x), y_(point.y) {
}
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
POINT Point::ToPOINT() const {
  POINT p;
  p.x = x();
  p.y = y();
  return p;
}
#elif defined(MINI_CHROMIUM_OS_MACOSX)
CGPoint Point::ToCGPoint() const {
  return CGPointMake(x(), y());
}
#endif

void Point::SetToMin(const Point& other) {
  x_ = x_ <= other.x_ ? x_ : other.x_;
  y_ = y_ <= other.y_ ? y_ : other.y_;
}

void Point::SetToMax(const Point& other) {
  x_ = x_ >= other.x_ ? x_ : other.x_;
  y_ = y_ >= other.y_ ? y_ : other.y_;
}

std::string Point::ToString() const {
  return cr::StringPrintf("%d,%d", x(), y());
}

Point ScaleToCeiledPoint(const Point& point, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return point;
  return ToCeiledPoint(ScalePoint(gfx::PointF(point), x_scale, y_scale));
}

Point ScaleToCeiledPoint(const Point& point, float scale) {
  if (scale == 1.f)
    return point;
  return ToCeiledPoint(ScalePoint(gfx::PointF(point), scale, scale));
}

Point ScaleToFlooredPoint(const Point& point, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return point;
  return ToFlooredPoint(ScalePoint(gfx::PointF(point), x_scale, y_scale));
}

Point ScaleToFlooredPoint(const Point& point, float scale) {
  if (scale == 1.f)
    return point;
  return ToFlooredPoint(ScalePoint(gfx::PointF(point), scale, scale));
}

Point ScaleToRoundedPoint(const Point& point, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return point;
  return ToRoundedPoint(ScalePoint(gfx::PointF(point), x_scale, y_scale));
}

Point ScaleToRoundedPoint(const Point& point, float scale) {
  if (scale == 1.f)
    return point;
  return ToRoundedPoint(ScalePoint(gfx::PointF(point), scale, scale));
}

}  // namespace gfx
}  // namespace crui

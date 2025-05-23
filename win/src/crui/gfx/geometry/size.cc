// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/size.h"

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#elif defined(MINI_CHROMIUM_OS_MACOSX)
#include <ApplicationServices/ApplicationServices.h>
#endif

#include "crbase/numerics/clamped_math.h"
#include "crbase/numerics/safe_math.h"
#include "crbase/strings/stringprintf.h"
#include "crui/gfx/geometry/safe_integer_conversions.h"
#include "crui/gfx/geometry/size_conversions.h"

namespace crui {
namespace gfx {

#if defined(MINI_CHROMIUM_OS_MACOSX)
Size::Size(const CGSize& s)
    : width_(s.width < 0 ? 0 : s.width),
      height_(s.height < 0 ? 0 : s.height) {
}

Size& Size::operator=(const CGSize& s) {
  set_width(s.width);
  set_height(s.height);
  return *this;
}
#endif

void Size::operator+=(const Size& size) {
  Enlarge(size.width(), size.height());
}

void Size::operator-=(const Size& size) {
  Enlarge(-size.width(), -size.height());
}

#if defined(MINI_CHROMIUM_OS_WIN)
SIZE Size::ToSIZE() const {
  SIZE s;
  s.cx = width();
  s.cy = height();
  return s;
}
#elif defined(MINI_CHROMIUM_OS_MACOSX)
CGSize Size::ToCGSize() const {
  return CGSizeMake(width(), height());
}
#endif

int Size::GetArea() const {
  return GetCheckedArea().ValueOrDie();
}

cr::CheckedNumeric<int> Size::GetCheckedArea() const {
  cr::CheckedNumeric<int> checked_area = width();
  checked_area *= height();
  return checked_area;
}

void Size::Enlarge(int grow_width, int grow_height) {
  SetSize(cr::ClampAdd(width(), grow_width),
          cr::ClampAdd(height(), grow_height));
}

void Size::SetToMin(const Size& other) {
  width_ = width() <= other.width() ? width() : other.width();
  height_ = height() <= other.height() ? height() : other.height();
}

void Size::SetToMax(const Size& other) {
  width_ = width() >= other.width() ? width() : other.width();
  height_ = height() >= other.height() ? height() : other.height();
}

std::string Size::ToString() const {
  return cr::StringPrintf("%dx%d", width(), height());
}

Size ScaleToCeiledSize(const Size& size, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return size;
  return ToCeiledSize(ScaleSize(gfx::SizeF(size), x_scale, y_scale));
}

Size ScaleToCeiledSize(const Size& size, float scale) {
  if (scale == 1.f)
    return size;
  return ToCeiledSize(ScaleSize(gfx::SizeF(size), scale, scale));
}

Size ScaleToFlooredSize(const Size& size, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return size;
  return ToFlooredSize(ScaleSize(gfx::SizeF(size), x_scale, y_scale));
}

Size ScaleToFlooredSize(const Size& size, float scale) {
  if (scale == 1.f)
    return size;
  return ToFlooredSize(ScaleSize(gfx::SizeF(size), scale, scale));
}

Size ScaleToRoundedSize(const Size& size, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return size;
  return ToRoundedSize(ScaleSize(gfx::SizeF(size), x_scale, y_scale));
}

Size ScaleToRoundedSize(const Size& size, float scale) {
  if (scale == 1.f)
    return size;
  return ToRoundedSize(ScaleSize(gfx::SizeF(size), scale, scale));
}

}  // namespace gfx
}  // namespace crui

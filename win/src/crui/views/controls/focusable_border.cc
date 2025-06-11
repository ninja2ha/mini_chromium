// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/controls/focusable_border.h"

#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "crui/gfx/canvas.h"
#include "crui/gfx/geometry/insets.h"
#include "crui/gfx/geometry/safe_integer_conversions.h"
#include "crui/gfx/scoped_canvas.h"
#include "crui/gfx/skia/skia_util.h"

namespace {

constexpr int kInsetSize = 1;
constexpr SkColor kDefaultColor = SK_ColorLTGRAY;

}  // namespace

namespace crui {
namespace views {

FocusableBorder::FocusableBorder() 
    : insets_(kInsetSize), color_(kDefaultColor) {}

FocusableBorder::~FocusableBorder() = default;

void FocusableBorder::Paint(const View& view, gfx::Canvas* canvas) {
  SkPaint paint;
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setColor(GetCurrentColor(view));

  gfx::ScopedCanvas scoped(canvas);
  float dsf = canvas->UndoDeviceScaleFactor();

  constexpr int kStrokeWidthPx = 1;
  paint.setStrokeWidth(SkIntToScalar(kStrokeWidthPx));

  // Scale the rect and snap to pixel boundaries.
  gfx::RectF rect(gfx::ScaleToEnclosedRect(view.GetLocalBounds(), dsf));
  rect.Inset(gfx::InsetsF(kStrokeWidthPx / 2.0f));

  SkPath path;
  paint.setAntiAlias(true);
  float corner_radius_px = kCornerRadiusDp * dsf;
  path.addRoundRect(gfx::RectFToSkRect(rect), corner_radius_px,
                    corner_radius_px);
  canvas->DrawPath(path, paint);
}

gfx::Insets FocusableBorder::GetInsets() const {
  return insets_;
}

gfx::Size FocusableBorder::GetMinimumSize() const {
  return gfx::Size();
}

void FocusableBorder::SetInsets(int top, int left, int bottom, int right) {
  insets_.Set(top, left, bottom, right);
}

void FocusableBorder::SetInsets(int vertical, int horizontal) {
  SetInsets(vertical, horizontal, vertical, horizontal);
}

void FocusableBorder::SetColor(cr::Optional<SkColor> color) {
  color_ = std::move(color);
}

SkColor FocusableBorder::GetCurrentColor(const View& view) const {
  return color_.value_or(kDefaultColor);
}

}  // namespace views
}  // namespace crui

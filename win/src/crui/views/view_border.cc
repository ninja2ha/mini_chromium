// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/view_border.h"

#include <memory>

#include "crbase/logging.h"
#include "crbase/memory/ptr_util.h"
#include "crui/gfx/canvas.h"
#include "crui/gfx/geometry/rect_f.h"
#include "crui/gfx/geometry/dip_util.h"
#include "crui/gfx/scoped_canvas.h"
#include "crui/gfx/skia_util.h"
///#include "crui/views/painter.h"
#include "crui/views/view.h"

namespace crui {
namespace views {

namespace {

// A simple border with different thicknesses on each side and single color.
class SolidSidedBorder : public Border {
 public:
  SolidSidedBorder(const SolidSidedBorder&) = delete;
  SolidSidedBorder& operator=(const SolidSidedBorder&) = delete;

  SolidSidedBorder(const gfx::Insets& insets, SkColor color);

  // Overridden from Border:
  void Paint(const View& view, gfx::Canvas* canvas) override;
  gfx::Insets GetInsets() const override;
  gfx::Size GetMinimumSize() const override;

 private:
  const gfx::Insets insets_;
};

SolidSidedBorder::SolidSidedBorder(const gfx::Insets& insets, SkColor color)
    : Border(color), insets_(insets) {}

void SolidSidedBorder::Paint(const View& view, gfx::Canvas* canvas) {
  // Undo DSF so that we can be sure to draw an integral number of pixels for
  // the border. Integral scale factors should be unaffected by this, but for
  // fractional scale factors this ensures sharp lines.
  gfx::ScopedCanvas scoped(canvas);
  float dsf = canvas->UndoDeviceScaleFactor();

  gfx::RectF scaled_bounds;
  if (view.layer()) {
    scaled_bounds =
        gfx::RectF(gfx::ConvertRectToPixel(view.layer()->device_scale_factor(), 
                   view.GetLocalBounds()));
  } else {
    scaled_bounds = gfx::RectF(view.GetLocalBounds());
    scaled_bounds.Scale(dsf);
  }

  // This scaling operation floors the inset values.
  scaled_bounds.Inset(insets_.Scale(dsf));
  canvas->sk_canvas()->clipRect(gfx::RectFToSkRect(scaled_bounds),
                                SkRegion::Op::kDifference_Op, 
                                true);
  canvas->DrawColor(color());
}

gfx::Insets SolidSidedBorder::GetInsets() const {
  return insets_;
}

gfx::Size SolidSidedBorder::GetMinimumSize() const {
  return gfx::Size(insets_.width(), insets_.height());
}

// A border with a rounded rectangle and single color.
class RoundedRectBorder : public Border {
 public:
  RoundedRectBorder(const RoundedRectBorder&) = delete;
  RoundedRectBorder& operator=(const RoundedRectBorder&) = delete;

  RoundedRectBorder(int thickness,
                    int corner_radius,
                    const gfx::Insets& paint_insets,
                    SkColor color);

  // Overridden from Border:
  void Paint(const View& view, gfx::Canvas* canvas) override;
  gfx::Insets GetInsets() const override;
  gfx::Size GetMinimumSize() const override;

 private:
  const int thickness_;
  const int corner_radius_;
  const gfx::Insets paint_insets_;
};

RoundedRectBorder::RoundedRectBorder(int thickness,
                                     int corner_radius,
                                     const gfx::Insets& paint_insets,
                                     SkColor color)
    : Border(color),
      thickness_(thickness),
      corner_radius_(corner_radius),
      paint_insets_(paint_insets) {}

void RoundedRectBorder::Paint(const View& view, gfx::Canvas* canvas) {
  SkPaint paint;
  paint.setStrokeWidth(SkIntToScalar(thickness_));
  paint.setColor(color());
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setAntiAlias(true);

  float half_thickness = thickness_ / 2.0f;
  gfx::RectF bounds(view.GetLocalBounds());
  bounds.Inset(paint_insets_);
  bounds.Inset(half_thickness, half_thickness);
  canvas->DrawRoundRect(bounds, SkIntToScalar(corner_radius_), paint);
}

gfx::Insets RoundedRectBorder::GetInsets() const {
  return gfx::Insets(thickness_) + paint_insets_;
}

gfx::Size RoundedRectBorder::GetMinimumSize() const {
  return gfx::Size(thickness_ * 2, thickness_ * 2);
}

class EmptyBorder : public Border {
 public:
  EmptyBorder(const EmptyBorder&) = delete;
  EmptyBorder& operator=(const EmptyBorder&) = delete;

  explicit EmptyBorder(const gfx::Insets& insets);

  // Overridden from Border:
  void Paint(const View& view, gfx::Canvas* canvas) override;
  gfx::Insets GetInsets() const override;
  gfx::Size GetMinimumSize() const override;

 private:
  const gfx::Insets insets_;
};

EmptyBorder::EmptyBorder(const gfx::Insets& insets) : insets_(insets) {
}

void EmptyBorder::Paint(const View& view, gfx::Canvas* canvas) {
}

gfx::Insets EmptyBorder::GetInsets() const {
  return insets_;
}

gfx::Size EmptyBorder::GetMinimumSize() const {
  return gfx::Size();
}

class ExtraInsetsBorder : public Border {
 public:
  ExtraInsetsBorder(const ExtraInsetsBorder&) = delete;
  ExtraInsetsBorder& operator=(const ExtraInsetsBorder&) = delete;

  ExtraInsetsBorder(std::unique_ptr<Border> border, const gfx::Insets& insets);

  // Overridden from Border:
  void Paint(const View& view, gfx::Canvas* canvas) override;
  gfx::Insets GetInsets() const override;
  gfx::Size GetMinimumSize() const override;

 private:
  std::unique_ptr<Border> border_;
  const gfx::Insets extra_insets_;
};

ExtraInsetsBorder::ExtraInsetsBorder(std::unique_ptr<Border> border,
                                     const gfx::Insets& insets)
    : border_(std::move(border)), extra_insets_(insets) {}

void ExtraInsetsBorder::Paint(const View& view, gfx::Canvas* canvas) {
  border_->Paint(view, canvas);
}

gfx::Insets ExtraInsetsBorder::GetInsets() const {
  return border_->GetInsets() + extra_insets_;
}

gfx::Size ExtraInsetsBorder::GetMinimumSize() const {
  gfx::Size size = border_->GetMinimumSize();
  size.Enlarge(extra_insets_.width(), extra_insets_.height());
  return size;
}

///class BorderPainter : public Border {
/// public:
///  BorderPainter(std::unique_ptr<Painter> painter, const gfx::Insets& insets);
///
///  // Overridden from Border:
///  void Paint(const View& view, gfx::Canvas* canvas) override;
///  gfx::Insets GetInsets() const override;
///  gfx::Size GetMinimumSize() const override;
///
/// private:
///  std::unique_ptr<Painter> painter_;
///  const gfx::Insets insets_;
///
///  DISALLOW_COPY_AND_ASSIGN(BorderPainter);
///};
///
///BorderPainter::BorderPainter(std::unique_ptr<Painter> painter,
///                             const gfx::Insets& insets)
///    : painter_(std::move(painter)), insets_(insets) {
///  DCHECK(painter_);
///}
///
///void BorderPainter::Paint(const View& view, gfx::Canvas* canvas) {
///  Painter::PaintPainterAt(canvas, painter_.get(), view.GetLocalBounds());
///}
///
///gfx::Insets BorderPainter::GetInsets() const {
///  return insets_;
///}
///
///gfx::Size BorderPainter::GetMinimumSize() const {
///  return painter_->GetMinimumSize();
///}

}  // namespace

Border::Border() = default;

Border::Border(SkColor color) : color_(color) {}

Border::~Border() = default;

std::unique_ptr<Border> NullBorder() {
  return nullptr;
}

std::unique_ptr<Border> CreateSolidBorder(int thickness, SkColor color) {
  return std::make_unique<SolidSidedBorder>(gfx::Insets(thickness), color);
}

std::unique_ptr<Border> CreateEmptyBorder(const gfx::Insets& insets) {
  return std::make_unique<EmptyBorder>(insets);
}

std::unique_ptr<Border> CreateRoundedRectBorder(int thickness,
                                                int corner_radius,
                                                SkColor color) {
  return CreateRoundedRectBorder(thickness, corner_radius, gfx::Insets(),
                                 color);
}
std::unique_ptr<Border> CreateRoundedRectBorder(int thickness,
                                                int corner_radius,
                                                const gfx::Insets& paint_insets,
                                                SkColor color) {
  return std::make_unique<RoundedRectBorder>(thickness, corner_radius,
                                             paint_insets, color);
}

std::unique_ptr<Border> CreateEmptyBorder(int top,
                                          int left,
                                          int bottom,
                                          int right) {
  return CreateEmptyBorder(gfx::Insets(top, left, bottom, right));
}

std::unique_ptr<Border> CreateSolidSidedBorder(int top,
                                               int left,
                                               int bottom,
                                               int right,
                                               SkColor color) {
  return std::make_unique<SolidSidedBorder>(
      gfx::Insets(top, left, bottom, right), color);
}

std::unique_ptr<Border> CreatePaddedBorder(std::unique_ptr<Border> border,
                                           const gfx::Insets& insets) {
  return std::make_unique<ExtraInsetsBorder>(std::move(border), insets);
}

///std::unique_ptr<Border> CreateBorderPainter(std::unique_ptr<Painter> painter,
///                                            const gfx::Insets& insets) {
///  return base::WrapUnique(new BorderPainter(std::move(painter), insets));
///}

}  // namespace views
}  // namespace crui

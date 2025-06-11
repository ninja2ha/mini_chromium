// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/controls/highlight_path_generator.h"

#include "third_party/skia/include/core/SkRect.h"
#include "crui/gfx/skia/skia_util.h"
#include "crui/views/view.h"
#include "crui/views/view_class_properties.h"

namespace crui {
namespace views {

HighlightPathGenerator::~HighlightPathGenerator() = default;

void HighlightPathGenerator::Install(
    View* host,
    std::unique_ptr<HighlightPathGenerator> generator) {
  host->SetProperty(kHighlightPathGeneratorKey, generator.release());
}

SkPath RectHighlightPathGenerator::GetHighlightPath(const View* view) {
  SkPath path;
  path.addRect(gfx::RectToSkRect(view->GetLocalBounds()));
  return path;
}

void InstallRectHighlightPathGenerator(View* view) {
  HighlightPathGenerator::Install(
      view, std::make_unique<RectHighlightPathGenerator>());
}

SkPath CircleHighlightPathGenerator::GetHighlightPath(const View* view) {
  const SkRect rect = gfx::RectToSkRect(view->GetLocalBounds());
  const SkScalar radius = SkScalarHalf(std::min(rect.width(), rect.height()));

  SkPath path;
  path.addCircle(rect.centerX(), rect.centerY(), radius);
  return path;
}

void InstallCircleHighlightPathGenerator(View* view) {
  HighlightPathGenerator::Install(
      view, std::make_unique<CircleHighlightPathGenerator>());
}

SkPath PillHighlightPathGenerator::GetHighlightPath(const View* view) {
  const SkRect rect = gfx::RectToSkRect(view->GetLocalBounds());
  const SkScalar radius = SkScalarHalf(std::min(rect.width(), rect.height()));

  SkPath path;
  path.addRoundRect(gfx::RectToSkRect(view->GetLocalBounds()),
                    radius, radius);
  return path;
}

void InstallPillHighlightPathGenerator(View* view) {
  HighlightPathGenerator::Install(
      view, std::make_unique<PillHighlightPathGenerator>());
}

}  // namespace views
}  // namespace crui

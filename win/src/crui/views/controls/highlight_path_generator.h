// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_HIGHLIGHT_PATH_GENERATOR_H_
#define UI_VIEWS_CONTROLS_HIGHLIGHT_PATH_GENERATOR_H_

#include <memory>

#include "third_party/skia/include/core/SkPath.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

class View;

// HighlightPathGenerators are used to generate its highlight path. This
// highlight path is used to generate the View's focus ring and ink-drop
// effects.
class CRUI_EXPORT HighlightPathGenerator {
 public:
  HighlightPathGenerator() = default;
  virtual ~HighlightPathGenerator();

  HighlightPathGenerator(const HighlightPathGenerator&) = delete;
  HighlightPathGenerator& operator=(const HighlightPathGenerator&) = delete;

  static void Install(View* host,
                      std::unique_ptr<HighlightPathGenerator> generator);

  virtual SkPath GetHighlightPath(const View* view) = 0;
};

// Sets a rectangular highlight path.
class CRUI_EXPORT RectHighlightPathGenerator : public HighlightPathGenerator {
 public:
  RectHighlightPathGenerator() = default;

  RectHighlightPathGenerator(const RectHighlightPathGenerator&) = delete;
  RectHighlightPathGenerator& operator=(const RectHighlightPathGenerator&) =
      delete;

  // HighlightPathGenerator:
  SkPath GetHighlightPath(const View* view) override;
};

void CRUI_EXPORT InstallRectHighlightPathGenerator(View* view);

// Sets a centered circular highlight path.
class CRUI_EXPORT CircleHighlightPathGenerator
    : public HighlightPathGenerator {
 public:
  CircleHighlightPathGenerator() = default;

  CircleHighlightPathGenerator(const CircleHighlightPathGenerator&) = delete;
  CircleHighlightPathGenerator& operator=(const CircleHighlightPathGenerator&) =
      delete;

  // HighlightPathGenerator:
  SkPath GetHighlightPath(const View* view) override;
};

void CRUI_EXPORT InstallCircleHighlightPathGenerator(View* view);

// Sets a pill-shaped highlight path.
class CRUI_EXPORT PillHighlightPathGenerator : public HighlightPathGenerator {
 public:
  PillHighlightPathGenerator() = default;

  PillHighlightPathGenerator(const PillHighlightPathGenerator&) = delete;
  PillHighlightPathGenerator& operator=(const PillHighlightPathGenerator&) =
      delete;

  // HighlightPathGenerator:
  SkPath GetHighlightPath(const View* view) override;
};

void CRUI_EXPORT InstallPillHighlightPathGenerator(View* view);

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_CONTROLS_HIGHLIGHT_PATH_GENERATOR_H_

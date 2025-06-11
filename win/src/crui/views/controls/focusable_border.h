// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_FOCUSABLE_BORDER_H_
#define UI_VIEWS_CONTROLS_FOCUSABLE_BORDER_H_

#include "crbase/compiler_specific.h"
#include "crbase/containers/optional.h"
#include "third_party/skia/include/core/SkColor.h"
#include "crui/views/view_border.h"
#include "crui/views/view.h"

namespace crui {

namespace gfx {
class Canvas;
class Insets;
}  // namespace gfx

namespace views {

// A Border class to draw a focused border around a field (e.g textfield).
class CRUI_EXPORT FocusableBorder : public Border {
 public:
  static constexpr float kCornerRadiusDp = 2.f;

  FocusableBorder(const FocusableBorder&) = delete;
  FocusableBorder& operator=(const FocusableBorder&) = delete;

  FocusableBorder();
  ~FocusableBorder() override;

  // Sets the insets of the border.
  void SetInsets(int top, int left, int bottom, int right);
  void SetInsets(int vertical, int horizontal);
  void SetColor(cr::Optional<SkColor> color);

  // Overridden from Border:
  void Paint(const View& view, gfx::Canvas* canvas) override;
  gfx::Insets GetInsets() const override;
  gfx::Size GetMinimumSize() const override;

 protected:
  SkColor GetCurrentColor(const View& view) const;

 private:
  gfx::Insets insets_;
  cr::Optional<SkColor> color_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_CONTROLS_FOCUSABLE_BORDER_H_

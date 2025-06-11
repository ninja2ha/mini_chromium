// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/style/typography.h"

#include "crbase/logging.h"
#include "crui/views/layout/layout_provider.h"
#include "crui/views/style/typography_provider.h"

namespace crui {

namespace views {
namespace style {

namespace {
void ValidateContextAndStyle(int context, int style) {
  CR_DCHECK(context >= VIEWS_TEXT_CONTEXT_START);
  CR_DCHECK(context < TEXT_CONTEXT_MAX);
  CR_DCHECK(style >= VIEWS_TEXT_STYLE_START);
}
}  // namespace

const gfx::FontList& GetFont(int context, int style) {
  ValidateContextAndStyle(context, style);
  return LayoutProvider::Get()->GetTypographyProvider().GetFont(context, style);
}

SkColor GetColor(const views::View& view, int context, int style) {
  ValidateContextAndStyle(context, style);
  return LayoutProvider::Get()->GetTypographyProvider().GetColor(view, context,
                                                                 style);
}

int GetLineHeight(int context, int style) {
  ValidateContextAndStyle(context, style);
  return LayoutProvider::Get()->GetTypographyProvider().GetLineHeight(context,
                                                                      style);
}

}  // namespace style
}  // namespace views

}  // namespace crui

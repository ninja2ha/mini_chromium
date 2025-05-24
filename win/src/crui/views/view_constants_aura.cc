// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/view_constants_aura.h"

#include "crui/base/class_property.h"
#include "crui/views/view.h"

DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, crui::views::View*)

namespace crui {
namespace views {

DEFINE_UI_CLASS_PROPERTY_KEY(crui::views::View*, kHostViewKey, NULL)

}  // namespace views
}  // namespace crui

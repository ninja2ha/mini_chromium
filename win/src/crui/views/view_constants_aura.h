// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_VIEW_CONSTANTS_AURA_H_
#define UI_VIEWS_VIEW_CONSTANTS_AURA_H_

#include "crui/aura/window.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {
class View;

// A property key to indicate the view the window is associated with. If
// specified, the z-order of the view, relative to other views, dictates the
// z-order of the window and its associated layer. The associated view must
// have the same parent widget as the window on which the property is set.
CRUI_EXPORT extern const aura::WindowProperty<View*>* const kHostViewKey;

}  // namespace views
}  // namespace crui

// Declaring the template specialization here to make sure that the
// compiler in all builds, including jumbo builds, always knows about
// the specialization before the first template instance use. Using a
// template instance before its specialization is declared in a
// translation unit is a C++ error.
DECLARE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, crui::views::View*)

#endif  // UI_VIEWS_VIEW_CONSTANTS_AURA_H_

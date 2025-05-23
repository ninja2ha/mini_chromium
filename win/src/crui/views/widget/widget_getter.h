// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_WIDGET_GETTER_H_
#define UI_VIEWS_WIDGET_WIDGET_GETTER_H_

#include "crbase/compiler_specific.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace views {
class Widget;

// Make this class a virtual base of any base class which needs to expose a
// GetWidget() method, then override GetWidgetImpl() as appropriate.
//
// Having GetWidgetImpl() be the override, not GetWidget() directly, avoids the
// need for subclasses to either override both GetWidget() methods or risk
// obscure compilation or name-hiding errors from only overriding one.  Using
// this as a virtual base avoids the need to explicitly qualify GetWidget()
// calls with a base class name when multiple bases expose it, as in e.g.
// View + WidgetDelegate.
class CRUI_EXPORT WidgetGetter {
 public:
  CR_NOINLINE Widget* GetWidget();
  CR_NOINLINE const Widget* GetWidget() const;

 private:
  virtual const Widget* GetWidgetImpl() const = 0;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_WIDGET_GETTER_H_

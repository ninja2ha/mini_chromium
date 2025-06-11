// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_WIDGET_DELETION_OBSERVER_H_
#define UI_VIEWS_WIDGET_WIDGET_DELETION_OBSERVER_H_

#include "crbase/compiler_specific.h"
#include "crui/base/ui_export.h"
#include "crui/views/widget/widget_observer.h"

namespace crui {

namespace views {
class Widget;

// A simple WidgetObserver that can be probed for the life of a widget.
class CRUI_EXPORT WidgetDeletionObserver : public WidgetObserver {
 public:
  WidgetDeletionObserver(const WidgetDeletionObserver&) = delete;
  WidgetDeletionObserver& operator=(const WidgetDeletionObserver&) = delete;

  explicit WidgetDeletionObserver(Widget* widget);
  ~WidgetDeletionObserver() override;

  // Returns true if the widget passed in the constructor is still alive.
  bool IsWidgetAlive() { return widget_ != nullptr; }

  // Overridden from WidgetObserver.
  void OnWidgetDestroying(Widget* widget) override;

 private:
  void CleanupWidget();

  Widget* widget_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_WIDGET_DELETION_OBSERVER_H_

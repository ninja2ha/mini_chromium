// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_VIEW_TRACKER_H_
#define UI_VIEWS_VIEW_TRACKER_H_

#include "crbase/scoped_observer.h"
#include "crui/views/view.h"
#include "crui/views/view_observer.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {
// ViewTracker tracks a single View. When the View is deleted it's removed.
class CRUI_EXPORT ViewTracker : public ViewObserver {
 public:
  explicit ViewTracker(View* view = nullptr);
  ViewTracker(const ViewTracker&) = delete;
  ViewTracker& operator=(const ViewTracker&) = delete;
  ~ViewTracker() override;

  void SetView(View* view);
  View* view() { return view_; }

  // ViewObserver:
  void OnViewIsDeleting(View* observed_view) override;

 private:
  View* view_ = nullptr;

  cr::ScopedObserver<View, ViewObserver> observer_{this};
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_VIEW_TRACKER_H_

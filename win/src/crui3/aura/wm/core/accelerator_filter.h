// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_ACCELERATOR_FILTER_H_
#define UI_WM_CORE_ACCELERATOR_FILTER_H_

#include <memory>

#include "crui/events/event_handler.h"
#include "crui/base/ui_export.h"

namespace crui {
class AcceleratorHistory;

namespace wm {
class AcceleratorDelegate;

// AcceleratorFilter filters key events for AcceleratorControler handling global
// keyboard accelerators.
class CRUI_EXPORT AcceleratorFilter : public crui::EventHandler {
 public:
  AcceleratorFilter(const AcceleratorFilter&) = delete;
  AcceleratorFilter& operator=(const AcceleratorFilter&) = delete;

  // AcceleratorFilter doesn't own |accelerator_history|, it's owned by
  // AcceleratorController.
  AcceleratorFilter(std::unique_ptr<AcceleratorDelegate> delegate,
                    crui::AcceleratorHistory* accelerator_history);
  ~AcceleratorFilter() override;

  // If the return value is true, |event| should be filtered out.
  static bool ShouldFilter(crui::KeyEvent* event);

  // Overridden from ui::EventHandler:
  void OnKeyEvent(crui::KeyEvent* event) override;
  void OnMouseEvent(crui::MouseEvent* event) override;

 private:
  std::unique_ptr<AcceleratorDelegate> delegate_;
  crui::AcceleratorHistory* accelerator_history_;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_ACCELERATOR_FILTER_H_

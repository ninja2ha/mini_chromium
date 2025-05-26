// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_CONTROLS_BUTTON_BUTTON_OBSERVER_H_
#define UI_VIEWS_CONTROLS_BUTTON_BUTTON_OBSERVER_H_

///#include "base/observer_list_types.h"
#include "crui/views/controls/button/button.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

class CRUI_EXPORT ButtonObserver /*: public base::CheckedObserver */{
 public:
  virtual void OnHighlightChanged(views::Button* observed_button,
                                  bool highlighted) {}

  virtual void OnStateChanged(views::Button* observed_button,
                              views::Button::ButtonState old_state) {}

 protected:
  ///~ButtonObserver() override = default;
   virtual ~ButtonObserver() = default;

};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_CONTROLS_BUTTON_BUTTON_OBSERVER_H_

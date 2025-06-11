// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/pointer/pointer_device.h"

namespace crui {

namespace {

int available_pointer_types_for_testing = POINTER_TYPE_NONE;
int available_hover_types_for_testing = HOVER_TYPE_NONE;
///bool return_available_pointer_and_hover_types_for_testing = false;
}  // namespace

std::pair<int, int> GetAvailablePointerAndHoverTypes() {
  ///if (return_available_pointer_and_hover_types_for_testing)
  ///  return std::make_pair(available_pointer_types_for_testing,
  ///                        available_hover_types_for_testing);
  return std::make_pair(GetAvailablePointerTypes(), GetAvailableHoverTypes());
}

}  // namespace crui

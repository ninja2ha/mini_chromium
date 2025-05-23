// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/display/display_observer.h"

namespace crui {
namespace display {

DisplayObserver::~DisplayObserver() {}

void DisplayObserver::OnWillProcessDisplayChanges() {}

void DisplayObserver::OnDidProcessDisplayChanges() {}

void DisplayObserver::OnDisplayAdded(const Display& new_display) {}

void DisplayObserver::OnDisplayRemoved(const Display& old_display) {}

void DisplayObserver::OnDisplayMetricsChanged(const Display& display,
                                              uint32_t changed_metrics) {}

}  // namespace display
}  // namespace crui

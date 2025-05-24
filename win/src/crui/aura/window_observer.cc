// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/window_observer.h"

#include "crbase/logging.h"

namespace crui {
namespace aura {

WindowObserver::WindowObserver() = default;

WindowObserver::~WindowObserver() {
  ///CR_CHECK(!IsInObserverList());
}

}  // namespace aura
}  // namespace crui

// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/platform/scoped_event_dispatcher.h"

#include "crui/events/platform/platform_event_source.h"

namespace crui {

ScopedEventDispatcher::ScopedEventDispatcher(
    PlatformEventDispatcher** scoped_dispatcher,
    PlatformEventDispatcher* new_dispatcher)
    : original_(*scoped_dispatcher),
      restore_(scoped_dispatcher, new_dispatcher) {}

ScopedEventDispatcher::~ScopedEventDispatcher() {
  PlatformEventSource::GetInstance()->OnOverriddenDispatcherRestored();
}

}  // namespace crui

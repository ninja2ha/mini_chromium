// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/event_injector.h"

#include <utility>

#include "crui/gfx/geometry/transform.h"
#include "crui/aura/window.h"
#include "crui/aura/window_tree_host.h"
#include "crui/events/event.h"
#include "crui/events/event_sink.h"

namespace crui {
namespace aura {

EventInjector::EventInjector() = default;

EventInjector::~EventInjector() = default;

crui::EventDispatchDetails EventInjector::Inject(WindowTreeHost* host,
                                                 crui::Event* event) {
  CR_DCHECK(host);
  CR_DCHECK(event);

  if (event->IsLocatedEvent()) {
    crui::LocatedEvent* located_event = event->AsLocatedEvent();
    // Transforming the coordinate to the root will apply the screen scale
    // factor to the event's location and also the screen rotation degree.
    located_event->UpdateForRootTransform(
        host->GetRootTransform(),
        host->GetRootTransformForLocalEventCoordinates());
  }

  return host->event_sink()->OnEventFromSource(event);
}

}  // namespace aura
}  // namespace crui

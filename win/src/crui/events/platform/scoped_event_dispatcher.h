// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_PLATFORM_SCOPED_EVENT_DISPATCHER_H_
#define UI_EVENTS_PLATFORM_SCOPED_EVENT_DISPATCHER_H_

#include "crbase/auto_reset.h"
#include "crui/base/ui_export.h"

namespace crui {

class PlatformEventDispatcher;

// A temporary PlatformEventDispatcher can be installed on a
// PlatformEventSource that overrides all installed event dispatchers, and
// always gets a chance to dispatch the event first. The PlatformEventSource
// returns a ScopedEventDispatcher object in such cases. This
// ScopedEventDispatcher object can be used to dispatch the event to any
// previous overridden dispatcher. When this object is destroyed, it removes the
// override-dispatcher, and restores the previous override-dispatcher.
class CRUI_EXPORT ScopedEventDispatcher {
 public:
  ScopedEventDispatcher(const ScopedEventDispatcher&) = delete;
  ScopedEventDispatcher& operator=(const ScopedEventDispatcher&) = delete;

  ScopedEventDispatcher(PlatformEventDispatcher** scoped_dispatcher,
                        PlatformEventDispatcher* new_dispatcher);
  ~ScopedEventDispatcher();

  operator PlatformEventDispatcher*() const { return original_; }

 private:
  PlatformEventDispatcher* original_;
  cr::AutoReset<PlatformEventDispatcher*> restore_;
};

}  // namespace crui

#endif  // UI_EVENTS_PLATFORM_SCOPED_EVENT_DISPATCHER_H_

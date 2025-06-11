// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_DRAGDROP_DROP_TARGET_EVENT_H_
#define UI_BASE_DRAGDROP_DROP_TARGET_EVENT_H_

#include "crui/base/dragdrop/os_exchange_data.h"
#include "crui/events/event.h"

namespace crui {

// Note: This object must not outlive the OSExchangeData used to construct it,
// as it stores that by reference.
class CRUI_EXPORT DropTargetEvent : public LocatedEvent {
 public:
  DropTargetEvent(const OSExchangeData& data,
                  const gfx::PointF& location,
                  const gfx::PointF& root_location,
                  int source_operations);
  DropTargetEvent(const DropTargetEvent& other);

  const OSExchangeData& data() const { return data_; }
  int source_operations() const { return source_operations_; }

 private:
  // Data associated with the drag/drop session.
  const OSExchangeData& data_;

  // Bitmask of supported DragDropTypes::DragOperation by the source.
  int source_operations_;
};

}  // namespace crui

#endif  // UI_BASE_DRAGDROP_DROP_TARGET_EVENT_H_


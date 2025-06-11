// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//core/accelerator_filter.h"

#include <utility>

#include "crui/base/accelerators/accelerator.h"
#include "crui/base/accelerators/accelerator_history.h"
#include "crui/events/event.h"
#include "crui/aura/wm//core/accelerator_delegate.h"
#include "crui/base/build_platform.h"

namespace crui {
namespace wm {

////////////////////////////////////////////////////////////////////////////////
// AcceleratorFilter, public:

AcceleratorFilter::AcceleratorFilter(
    std::unique_ptr<AcceleratorDelegate> delegate,
    crui::AcceleratorHistory* accelerator_history)
    : delegate_(std::move(delegate)),
      accelerator_history_(accelerator_history) {
  CR_DCHECK(accelerator_history);
}

AcceleratorFilter::~AcceleratorFilter() {
}

bool AcceleratorFilter::ShouldFilter(crui::KeyEvent* event) {
  const crui::EventType type = event->type();
  if (!event->target() ||
      (type != crui::ET_KEY_PRESSED && type != crui::ET_KEY_RELEASED) ||
      event->is_char() || !event->target() ||
      // Key events with key code of VKEY_PROCESSKEY, usually created by virtual
      // keyboard (like handwriting input), have no effect on accelerator and
      // they may disturb the accelerator history. So filter them out. (see
      // https://crbug.com/918317)
      event->key_code() == crui::VKEY_PROCESSKEY) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// AcceleratorFilter, EventFilter implementation:

void AcceleratorFilter::OnKeyEvent(crui::KeyEvent* event) {
  CR_DCHECK(event->target());
  if (ShouldFilter(event))
    return;

  crui::Accelerator accelerator(*event);
  accelerator_history_->StoreCurrentAccelerator(accelerator);

  if (delegate_->ProcessAccelerator(*event, accelerator))
    event->StopPropagation();
}

void AcceleratorFilter::OnMouseEvent(crui::MouseEvent* event) {
  if (event->type() == crui::ET_MOUSE_PRESSED ||
      event->type() == crui::ET_MOUSE_RELEASED) {
    accelerator_history_->InterruptCurrentAccelerator();
  }
}

}  // namespace wm
}  // namespace crui

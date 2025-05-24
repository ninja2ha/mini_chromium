// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//public/scoped_tooltip_disabler.h"

#include "crui/aura/window.h"
#include "crui/aura/wm//public/tooltip_client.h"

namespace crui {
namespace wm {

ScopedTooltipDisabler::ScopedTooltipDisabler(aura::Window* window)
    : root_(window ? window->GetRootWindow() : NULL) {
  if (root_) {
    root_->AddObserver(this);
    TooltipClient* client = GetTooltipClient(root_);
    if (client)
      client->SetTooltipsEnabled(false);
  }
}

ScopedTooltipDisabler::~ScopedTooltipDisabler() {
  EnableTooltips();
}

void ScopedTooltipDisabler::EnableTooltips() {
  if (!root_)
    return;
  TooltipClient* client = GetTooltipClient(root_);
  if (client)
    client->SetTooltipsEnabled(true);
  root_->RemoveObserver(this);
  root_ = NULL;
}

void ScopedTooltipDisabler::OnWindowDestroying(aura::Window* window) {
  EnableTooltips();
}

}  // namespace wm
}  // namespace crui

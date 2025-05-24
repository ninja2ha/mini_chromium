// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//public/tooltip_client.h"

#include "crui/aura/window.h"
#include "crui/base/class_property.h"

DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, crui::wm::TooltipClient*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, void**)

namespace crui {
namespace wm {

DEFINE_UI_CLASS_PROPERTY_KEY(TooltipClient*, kRootWindowTooltipClientKey, NULL)
DEFINE_UI_CLASS_PROPERTY_KEY(cr::string16*, kTooltipTextKey, NULL)
DEFINE_UI_CLASS_PROPERTY_KEY(void*, kTooltipIdKey, NULL)

void SetTooltipClient(aura::Window* root_window, TooltipClient* client) {
  CR_DCHECK(root_window->GetRootWindow() == root_window);
  root_window->SetProperty(kRootWindowTooltipClientKey, client);
}

TooltipClient* GetTooltipClient(aura::Window* root_window) {
  if (root_window)
    CR_DCHECK(root_window->GetRootWindow() == root_window);
  return root_window ?
      root_window->GetProperty(kRootWindowTooltipClientKey) : NULL;
}

void SetTooltipText(aura::Window* window, cr::string16* tooltip_text) {
  window->SetProperty(kTooltipTextKey, tooltip_text);
}

void SetTooltipId(aura::Window* window, void* id) {
  if (id != GetTooltipId(window))
    window->SetProperty(kTooltipIdKey, id);
}

const cr::string16 GetTooltipText(aura::Window* window) {
  cr::string16* string_ptr = window->GetProperty(kTooltipTextKey);
  return string_ptr ? *string_ptr : cr::string16();
}

const void* GetTooltipId(aura::Window* window) {
  return window->GetProperty(kTooltipIdKey);
}

}  // namespace wm
}  // namespace crui

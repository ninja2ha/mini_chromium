// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/keycodes/dom/dom_keyboard_layout_map_base.h"

#include <cstdint>
#include <memory>
#include <string>

#include "crbase/logging.h"
#include "crui/events/keycodes/dom/dom_key.h"
#include "crui/events/keycodes/dom/dom_keyboard_layout.h"
#include "crui/events/keycodes/dom/dom_keyboard_layout_manager.h"

namespace crui {

DomKeyboardLayoutMapBase::DomKeyboardLayoutMapBase() = default;

DomKeyboardLayoutMapBase::~DomKeyboardLayoutMapBase() = default;

cr::flat_map<std::string, std::string> DomKeyboardLayoutMapBase::Generate() {
  uint32_t keyboard_layout_count = GetKeyboardLayoutCount();
  if (!keyboard_layout_count)
    return {};

  std::unique_ptr<crui::DomKeyboardLayoutManager> keyboard_layout_manager =
      std::make_unique<crui::DomKeyboardLayoutManager>();

  for (uint32_t i = 0; i < keyboard_layout_count; i++) {
    DomKeyboardLayout* const dom_keyboard_layout =
        keyboard_layout_manager->GetLayout(i);
    PopulateLayout(i, dom_keyboard_layout);

    if (dom_keyboard_layout->IsAsciiCapable())
      return dom_keyboard_layout->GetMap();
  }

  return keyboard_layout_manager->GetFirstAsciiCapableLayout()->GetMap();
}

void DomKeyboardLayoutMapBase::PopulateLayout(uint32_t keyboard_layout_index,
                                              crui::DomKeyboardLayout* layout) {
  CR_DCHECK(layout);

  for (size_t entry = 0; entry < crui::kWritingSystemKeyDomCodeEntries; 
          entry++) {
    crui::DomCode dom_code = crui::writing_system_key_domcodes[entry];

    crui::DomKey dom_key =
        GetDomKeyFromDomCodeForLayout(dom_code, keyboard_layout_index);
    if (dom_key == crui::DomKey::NONE)
      continue;

    uint32_t unicode_value = 0;
    if (dom_key.IsCharacter())
      unicode_value = dom_key.ToCharacter();
    else if (dom_key.IsDeadKey())
      unicode_value = dom_key.ToDeadKeyCombiningCharacter();
    else if (dom_key == crui::DomKey::ZENKAKU_HANKAKU)
      // Placeholder for hankaku/zenkaku string.
      unicode_value = kHankakuZenkakuPlaceholder;

    if (unicode_value != 0)
      layout->AddKeyMapping(dom_code, unicode_value);
  }
}

}  // namespace crui

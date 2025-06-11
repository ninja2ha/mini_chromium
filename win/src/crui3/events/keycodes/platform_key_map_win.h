// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYCODES_PLATFORM_KEY_MAP_WIN_H_
#define UI_EVENTS_KEYCODES_PLATFORM_KEY_MAP_WIN_H_

#include <windows.h>

#include <unordered_map>

#include "crbase/hash/hash.h"
#include "crui/base/ui_export.h"
#include "crui/events/event.h"
#include "crui/events/keycodes/dom/dom_key.h"
#include "crui/events/keycodes/keyboard_codes_win.h"
#include "crui/events/platform_event.h"

namespace crui {

class CRUI_EXPORT PlatformKeyMap {
 public:
  PlatformKeyMap(const PlatformKeyMap&) = delete;
  PlatformKeyMap& operator=(const PlatformKeyMap&) = delete;

  // Create and load key map table with specified keyboard layout.
  // Visible for testing.
  explicit PlatformKeyMap(HKL layout);
  ~PlatformKeyMap();

  // Returns the DOM KeyboardEvent key from |KeyboardCode|+|EventFlags| and
  // the keyboard layout of current thread. If this is an AltGraph modified
  // key then |flags| will have Control+Alt removed, and AltGraph set.
  // Updates a per-thread key map cache whenever the layout changes.
  static DomKey DomKeyFromKeyboardCode(KeyboardCode key_code, int* flags);

  // Returns true if the currently-active keymap uses AltGraph shift.
  static bool UsesAltGraph();

  // If the supplied event has both Control and Alt modifiers set, then they
  // are replaced by AltGraph. This should only ever be applied to the flags
  // for printable-character events.
  static int ReplaceControlAndAltWithAltGraph(int flags);

 private:
  friend class PlatformKeyMapTest;

  PlatformKeyMap();

  // Returns the PlatformKeyMap instance for the current thread.
  static PlatformKeyMap* GetThreadLocalPlatformKeyMap();

  // TODO(input-dev): Expose this function when we need to access separate
  // layout. Returns the DomKey 'meaning' of |KeyboardCode| in the context of
  // specified |EventFlags| and stored keyboard layout.
  DomKey DomKeyFromKeyboardCodeImpl(KeyboardCode, int* flags) const;

  // TODO(input-dev): Expose this function in response to WM_INPUTLANGCHANGE.
  void UpdateLayout(HKL layout);

  HKL keyboard_layout_ = 0;

  // True if |keyboard_layout_| makes use of the AltGraph modifier.
  bool has_alt_graph_ = false;

  typedef std::pair<int /*KeyboardCode*/, int /*EventFlags*/>
      KeyboardCodeEventFlagsPair;
  typedef std::unordered_map<KeyboardCodeEventFlagsPair,
                             DomKey,
                             cr::IntPairHash<std::pair<int, int>>>
      KeyboardCodeToKeyMap;
  KeyboardCodeToKeyMap printable_keycode_to_key_;
};

}  // namespace crui

#endif  // UI_EVENTS_KEYCODES_PLATFORM_KEY_MAP_WIN_H_

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYCODES_KEYBOARD_CODE_CONVERSION_H_
#define UI_EVENTS_KEYCODES_KEYBOARD_CODE_CONVERSION_H_

#include "crbase/compiler_specific.h"
#include "crbase/strings/string16.h"
#include "crui/base/ui_export.h"
#include "crui/events/keycodes/dom/dom_key.h"
#include "crui/events/keycodes/keyboard_codes.h"

namespace crui {

enum class DomCode;

// Helper functions to get the meaning of a DOM |code| in a
// platform independent way. It supports control characters as well.
// It assumes a US keyboard layout is used, so it may only be used when there
// is no native event or no better way to get the character.
//
// For example, if a virtual keyboard implementation can only generate key
// events with key_code and flags information, then there is no way for us to
// determine the actual character that should be generate by the key. Because
// a key_code only represents a physical key on the keyboard, it has nothing
// to do with the actual character printed on that key. In such case, the only
// thing we can do is to assume that we are using a US keyboard and get the
// character according to US keyboard layout definition. Preferably, such
// events should be created using a full KeyEvent constructor, explicitly
// specifying the character and DOM 3 values as well as the legacy VKEY.

// Helper function to map a physical key state (dom_code and flags)
// to a meaning (dom_key and character, together corresponding to the
// DOM keyboard event |key| value), along with a corresponding non-located
// Windows-based key_code.
//
// This follows a US keyboard layout, so it should only be used when there
// is no other better way to obtain the meaning (e.g. actual keyboard layout).
// Returns true and sets the output parameters if the (dom_code, flags) pair
// has an interpretation in the US English layout; otherwise the output
// parameters are untouched.
CRUI_EXPORT cr::char16 DomCodeToUsLayoutCharacter(DomCode dom_code,
                                                  int flags);
CRUI_EXPORT bool DomCodeToUsLayoutDomKey(DomCode dom_code,
                                         int flags,
                                         DomKey* dom_key,
                                         KeyboardCode* key_code)
    CR_WARN_UNUSED_RESULT;

// Helper function to map a physical key (dom_code) to a meaning (dom_key
// and character, together corresponding to the DOM keyboard event |key|
// value), along with a corresponding non-located Windows-based key_code.
// Unlike |DomCodeToUsLayoutDomKey| this function only maps non-printable,
// or action, keys.
CRUI_EXPORT bool DomCodeToNonPrintableDomKey(DomCode dom_code,
                                             DomKey* dom_key,
                                             KeyboardCode* key_code)
    CR_WARN_UNUSED_RESULT;

// Obtains the control character corresponding to a physical key;
// that is, the meaning of the physical key state (dom_code, and flags
// containing EF_CONTROL_DOWN) under the base US English layout.
// Returns true and sets the output parameters if the (dom_code, flags) pair
// is interpreted as a control character; otherwise the output parameters
// are untouched.
CRUI_EXPORT bool DomCodeToControlCharacter(DomCode dom_code,
                                           int flags,
                                           DomKey* dom_key,
                                           KeyboardCode* key_code)
    CR_WARN_UNUSED_RESULT;

// Returns a Windows-based VKEY for a non-printable DOM Level 3 |key|.
// The returned VKEY is non-located (e.g. VKEY_SHIFT).
CRUI_EXPORT KeyboardCode
NonPrintableDomKeyToKeyboardCode(DomKey dom_key);

// Determine the non-located VKEY corresponding to a located VKEY.
// Most modifier keys have two kinds of KeyboardCode: located (e.g.
// VKEY_LSHIFT and VKEY_RSHIFT), that indentify one of two specific
// physical keys, and non-located (e.g. VKEY_SHIFT) that identify
// only the operation. Similarly digit keys have a number-pad variant
// (e.g. VKEY_NUMPAD1 on the number pad vs VKEY_1 on the main keyboard),
// except that in this case the main keyboard code doubles as the
// non-located value.
CRUI_EXPORT KeyboardCode
LocatedToNonLocatedKeyboardCode(KeyboardCode key_code);

// Determine the located VKEY corresponding to a non-located VKEY.
CRUI_EXPORT KeyboardCode
NonLocatedToLocatedKeyboardCode(KeyboardCode key_code, DomCode dom_code);

// Returns a DOM Level 3 |code| from a Windows-based VKEY value.
// This assumes a US layout and should only be used when |code| cannot be
// determined from a physical scan code, for example when a key event was
// generated synthetically by JavaScript with only a VKEY value supplied.
CRUI_EXPORT DomCode UsLayoutKeyboardCodeToDomCode(KeyboardCode key_code);

// Returns the Windows-based VKEY value corresponding to a DOM Level 3 |code|,
// assuming a base US English layout. The returned VKEY is located
// (e.g. VKEY_LSHIFT).
CRUI_EXPORT KeyboardCode DomCodeToUsLayoutKeyboardCode(DomCode dom_code);

// Returns the Windows-based VKEY value corresponding to a DOM Level 3 |code|,
// assuming a base US English layout. The returned VKEY is non-located
// (e.g. VKEY_SHIFT).
CRUI_EXPORT KeyboardCode
DomCodeToUsLayoutNonLocatedKeyboardCode(DomCode dom_code);

// Returns the ui::EventFlags value associated with a modifier key,
// or 0 (EF_NONE) if the key is not a modifier.
CRUI_EXPORT int ModifierDomKeyToEventFlag(DomKey key);

}  // namespace crui

#endif  // UI_EVENTS_KEYCODES_KEYBOARD_CODE_CONVERSION_H_

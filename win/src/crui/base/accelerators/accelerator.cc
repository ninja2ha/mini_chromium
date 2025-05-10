// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/accelerators/accelerator.h"

#include <stdint.h>
#include <tuple>

///#include "crbase/i18n/rtl.h"
#include "crbase/logging.h"
#include "crbase/strings/strcat.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/utf_string_conversions.h"
///#include "crui/base/l10n/l10n_util.h"
#include "crui/events/event.h"
#include "crui/events/keycodes/keyboard_code_conversion.h"
///#include "crui/strings/grit/ui_strings.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#endif

#if !defined(MINI_CHROMIUM_OS_WIN) && \
    (defined(MINI_CHROMIUM_USE_AURA) || defined(MINI_CHROMIUM_OS_MACOSX))
#include "crui/events/keycodes/keyboard_code_conversion.h"
#endif

namespace crui {

namespace {

const int kModifierMask = EF_SHIFT_DOWN | EF_CONTROL_DOWN | EF_ALT_DOWN |
                          EF_COMMAND_DOWN | EF_ALTGR_DOWN;

const int kInterestingFlagsMask =
    kModifierMask | EF_IS_SYNTHESIZED | EF_IS_REPEAT;

///cr::string16 ApplyModifierToAcceleratorString(
///    const cr::string16& accelerator,
///    int modifier_message_id) {
///  return l10n_util::GetStringFUTF16(
///      IDS_APP_ACCELERATOR_WITH_MODIFIER,
///      l10n_util::GetStringUTF16(modifier_message_id), accelerator);
///}

}  // namespace

Accelerator::Accelerator() : Accelerator(VKEY_UNKNOWN, EF_NONE) {}

Accelerator::Accelerator(KeyboardCode key_code,
                         int modifiers,
                         KeyState key_state,
                         cr::TimeTicks time_stamp)
    : key_code_(key_code),
      key_state_(key_state),
      modifiers_(modifiers & kInterestingFlagsMask),
      time_stamp_(time_stamp),
      interrupted_by_mouse_event_(false) {}

Accelerator::Accelerator(const KeyEvent& key_event)
    : key_code_(key_event.key_code()),
      key_state_(key_event.type() == ET_KEY_PRESSED ? KeyState::PRESSED
                                                    : KeyState::RELEASED),
      // |modifiers_| may include the repeat flag.
      modifiers_(key_event.flags() & kInterestingFlagsMask),
      time_stamp_(key_event.time_stamp()),
      interrupted_by_mouse_event_(false),
      source_device_id_(key_event.source_device_id()) {}

Accelerator::Accelerator(const Accelerator& accelerator) {
  key_code_ = accelerator.key_code_;
  key_state_ = accelerator.key_state_;
  modifiers_ = accelerator.modifiers_;
  time_stamp_ = accelerator.time_stamp_;
  interrupted_by_mouse_event_ = accelerator.interrupted_by_mouse_event_;
  source_device_id_ = accelerator.source_device_id_;
}

Accelerator::~Accelerator() {
}

// static
int Accelerator::MaskOutKeyEventFlags(int flags) {
  return flags & kModifierMask;
}

KeyEvent Accelerator::ToKeyEvent() const {
  return KeyEvent(key_state() == Accelerator::KeyState::PRESSED
                      ? ET_KEY_PRESSED
                      : ET_KEY_RELEASED,
                  key_code(), modifiers(), time_stamp());
}

Accelerator& Accelerator::operator=(const Accelerator& accelerator) {
  if (this != &accelerator) {
    key_code_ = accelerator.key_code_;
    key_state_ = accelerator.key_state_;
    modifiers_ = accelerator.modifiers_;
    time_stamp_ = accelerator.time_stamp_;
    interrupted_by_mouse_event_ = accelerator.interrupted_by_mouse_event_;
  }
  return *this;
}

bool Accelerator::operator <(const Accelerator& rhs) const {
  const int modifiers_with_mask = MaskOutKeyEventFlags(modifiers_);
  const int rhs_modifiers_with_mask = MaskOutKeyEventFlags(rhs.modifiers_);
  return std::tie(key_code_, key_state_, modifiers_with_mask) <
         std::tie(rhs.key_code_, rhs.key_state_, rhs_modifiers_with_mask);
}

bool Accelerator::operator ==(const Accelerator& rhs) const {
  return (key_code_ == rhs.key_code_) && (key_state_ == rhs.key_state_) &&
         (MaskOutKeyEventFlags(modifiers_) ==
          MaskOutKeyEventFlags(rhs.modifiers_)) &&
         interrupted_by_mouse_event_ == rhs.interrupted_by_mouse_event_;
}

bool Accelerator::operator !=(const Accelerator& rhs) const {
  return !(*this == rhs);
}

bool Accelerator::IsShiftDown() const {
  return (modifiers_ & EF_SHIFT_DOWN) != 0;
}

bool Accelerator::IsCtrlDown() const {
  return (modifiers_ & EF_CONTROL_DOWN) != 0;
}

bool Accelerator::IsAltDown() const {
  return (modifiers_ & EF_ALT_DOWN) != 0;
}

bool Accelerator::IsAltGrDown() const {
  return (modifiers_ & EF_ALTGR_DOWN) != 0;
}

bool Accelerator::IsCmdDown() const {
  return (modifiers_ & EF_COMMAND_DOWN) != 0;
}

bool Accelerator::IsRepeat() const {
  return (modifiers_ & EF_IS_REPEAT) != 0;
}

cr::string16 Accelerator::GetShortcutText() const {
  cr::string16 shortcut;

#if defined(MINI_CHROMIUM_OS_MACOSX)
  shortcut = KeyCodeToMacSymbol();
#else
  shortcut = KeyCodeToName();
#endif

  if (shortcut.empty()) {
#if defined(MINI_CHROMIUM_OS_WIN)
    // Our fallback is to try translate the key code to a regular character
    // unless it is one of digits (VK_0 to VK_9). Some keyboard
    // layouts have characters other than digits assigned in
    // an unshifted mode (e.g. French AZERY layout has 'a with grave
    // accent' for '0'). For display in the menu (e.g. Ctrl-0 for the
    // default zoom level), we leave VK_[0-9] alone without translation.
    wchar_t key;
    if (cr::IsAsciiDigit(key_code_))
      key = static_cast<wchar_t>(key_code_);
    else
      key = LOWORD(::MapVirtualKeyW(key_code_, MAPVK_VK_TO_CHAR));
    // If there is no translation for the given |key_code_| (e.g.
    // VKEY_UNKNOWN), |::MapVirtualKeyW| returns 0.
    if (key != 0)
      shortcut += key;
#elif defined(MINI_CHROMIUM_USE_AURA) || defined(MINI_CHROMIUM_OS_MACOSX)
    const uint16_t c = DomCodeToUsLayoutCharacter(
        UsLayoutKeyboardCodeToDomCode(key_code_), false);
    if (c != 0)
      shortcut +=
          static_cast<base::string16::value_type>(base::ToUpperASCII(c));
#endif
  }

#if defined(MINI_CHROMIUM_OS_MACOSX)
  shortcut = ApplyShortFormModifiers(shortcut);
#else
  // Checking whether the character used for the accelerator is alphanumeric.
  // If it is not, then we need to adjust the string later on if the locale is
  // right-to-left. See below for more information of why such adjustment is
  // required.
  cr::string16 shortcut_rtl;
  bool adjust_shortcut_for_rtl = false;
  ///if (cr::i18n::IsRTL() && shortcut.length() == 1 &&
  ///    !cr::IsAsciiAlpha(shortcut[0]) && !cr::IsAsciiDigit(shortcut[0])) {
  ///  adjust_shortcut_for_rtl = true;
  ///  shortcut_rtl.assign(shortcut);
  ///}

  shortcut = ApplyLongFormModifiers(shortcut);

  // For some reason, menus in Windows ignore standard Unicode directionality
  // marks (such as LRE, PDF, etc.). On RTL locales, we use RTL menus and
  // therefore any text we draw for the menu items is drawn in an RTL context.
  // Thus, the text "Ctrl++" (which we currently use for the Zoom In option)
  // appears as "++Ctrl" in RTL because the Unicode BiDi algorithm puts
  // punctuations on the left when the context is right-to-left. Shortcuts that
  // do not end with a punctuation mark (such as "Ctrl+H" do not have this
  // problem).
  //
  // The only way to solve this problem is to adjust the string if the locale
  // is RTL so that it is drawn correctly in an RTL context. Instead of
  // returning "Ctrl++" in the above example, we return "++Ctrl". This will
  // cause the text to appear as "Ctrl++" when Windows draws the string in an
  // RTL context because the punctuation no longer appears at the end of the
  // string.
  //
  // TODO(idana) bug# 1232732: this hack can be avoided if instead of using
  // views::Menu we use views::MenuItemView because the latter is a View
  // subclass and therefore it supports marking text as RTL or LTR using
  // standard Unicode directionality marks.
  if (adjust_shortcut_for_rtl) {
    int key_length = static_cast<int>(shortcut_rtl.length());
    CR_DCHECK(key_length > 0);
    shortcut_rtl.append(cr::ASCIIToUTF16("+"));

    // Subtracting the size of the shortcut key and 1 for the '+' sign.
    shortcut_rtl.append(shortcut, 0, shortcut.length() - key_length - 1);
    shortcut.swap(shortcut_rtl);
  }
#endif  // MINI_CHROMIUM_OS_MACOSX

  return shortcut;
}

#if defined(MINI_CHROMIUM_OS_MACOSX)
cr::string16 Accelerator::KeyCodeToMacSymbol() const {
  switch (key_code_) {
    case VKEY_CAPITAL:
      return cr::string16({0x21ea});
    case VKEY_RETURN:
      return cr::string16({0x2324});
    case VKEY_BACK:
      return cr::string16({0x232b});
    case VKEY_ESCAPE:
      return cr::string16({0x238b});
    case VKEY_RIGHT:
      return cr::string16({0x2192});
    case VKEY_LEFT:
      return cr::string16({0x2190});
    case VKEY_UP:
      return cr::string16({0x2191});
    case VKEY_DOWN:
      return cr::string16({0x2193});
    case VKEY_PRIOR:
      return cr::string16({0x21de});
    case VKEY_NEXT:
      return cr::string16({0x21df});
    case VKEY_HOME:
      return cr::string16({0x2196});
    case VKEY_END:
      return cr::string16({0x2198});
    case VKEY_TAB:
      return cr::string16({0x21e5});
    // Mac has a shift-tab icon (0x21e4) but we don't use it.
    // "Space" and some other keys are written out; fall back to KeyCodeToName()
    // for those (and any other unhandled keys).
    default:
      return KeyCodeToName();
  }
}
#endif  // MINI_CHROMIUM_OS_MACOSX

cr::string16 Accelerator::KeyCodeToName() const {
  ///int string_id = 0;
  switch (key_code_) {
    case VKEY_TAB:
      ///string_id = IDS_APP_TAB_KEY;
      return cr::ASCIIToUTF16("Tab");
      break;
    case VKEY_RETURN:
      ///string_id = IDS_APP_ENTER_KEY;
      return cr::ASCIIToUTF16("Enter");
      break;
    case VKEY_SPACE:
      ///string_id = IDS_APP_SPACE_KEY;
      return cr::ASCIIToUTF16("Space");
      break;
    case VKEY_PRIOR:
      ///string_id = IDS_APP_PAGEUP_KEY;
      return cr::ASCIIToUTF16("Page Up");
      break;
    case VKEY_NEXT:
      ///string_id = IDS_APP_PAGEDOWN_KEY;
      return cr::ASCIIToUTF16("Page Down");
      break;
    case VKEY_END:
      ///string_id = IDS_APP_END_KEY;
      return cr::ASCIIToUTF16("End");
      break;
    case VKEY_HOME:
      ///string_id = IDS_APP_HOME_KEY;
      return cr::ASCIIToUTF16("Home");
      break;
    case VKEY_INSERT:
      ///string_id = IDS_APP_INSERT_KEY;
      return cr::ASCIIToUTF16("Ins");
      break;
    case VKEY_DELETE:
      ///string_id = IDS_APP_DELETE_KEY;
      return cr::ASCIIToUTF16("Del");
      break;
    case VKEY_LEFT:
      ///string_id = IDS_APP_LEFT_ARROW_KEY;
      return cr::ASCIIToUTF16("Left Arrow");
      break;
    case VKEY_RIGHT:
      ///string_id = IDS_APP_RIGHT_ARROW_KEY;
      return cr::ASCIIToUTF16("Right Arrow");
      break;
    case VKEY_UP:
      ///string_id = IDS_APP_UP_ARROW_KEY;
      return cr::ASCIIToUTF16("Up Arrow");
      break;
    case VKEY_DOWN:
      ///string_id = IDS_APP_DOWN_ARROW_KEY;
      return cr::ASCIIToUTF16("Down Arrow");
      break;
    case VKEY_ESCAPE:
      ///string_id = IDS_APP_ESC_KEY;
      return cr::ASCIIToUTF16("Esc");
      break;
    case VKEY_BACK:
      ///string_id = IDS_APP_BACKSPACE_KEY;
      return cr::ASCIIToUTF16("Backspace");
      break;
    case VKEY_F1:
      ///string_id = IDS_APP_F1_KEY;
      return cr::ASCIIToUTF16("F1");
      break;
    case VKEY_F11:
      ///string_id = IDS_APP_F11_KEY;
      return cr::ASCIIToUTF16("F11");
      break;
#if !defined(MINI_CHROMIUM_OS_MACOSX)
    // On Mac, commas and periods are used literally in accelerator text.
    case VKEY_OEM_COMMA:
      ///string_id = IDS_APP_COMMA_KEY;
      return cr::ASCIIToUTF16("Comma");
      break;
    case VKEY_OEM_PERIOD:
      return cr::ASCIIToUTF16("Period");
      ///string_id = IDS_APP_PERIOD_KEY;
      break;
#endif
    case VKEY_MEDIA_NEXT_TRACK:
      return cr::ASCIIToUTF16("Media Next Track");
      ///string_id = IDS_APP_MEDIA_NEXT_TRACK_KEY;
      break;
    case VKEY_MEDIA_PLAY_PAUSE:
      return cr::ASCIIToUTF16("Media Play/Pause");
      ///string_id = IDS_APP_MEDIA_PLAY_PAUSE_KEY;
      break;
    case VKEY_MEDIA_PREV_TRACK:
      return cr::ASCIIToUTF16("Media Previous Track");
      ///string_id = IDS_APP_MEDIA_PREV_TRACK_KEY;
      break;
    case VKEY_MEDIA_STOP:
      return cr::ASCIIToUTF16("Media Stop");
      ///string_id = IDS_APP_MEDIA_STOP_KEY;
      break;
    default:
      break;
  }
  ///return string_id ? l10n_util::GetStringUTF16(string_id) : cr::string16();
  return cr::string16();
}

cr::string16 Accelerator::ApplyLongFormModifiers(
    cr::string16 shortcut) const {
///  if (IsShiftDown())
///    shortcut = ApplyModifierToAcceleratorString(shortcut, IDS_APP_SHIFT_KEY);
///
///  // Note that we use 'else-if' in order to avoid using Ctrl+Alt as a shortcut.
///  // See http://blogs.msdn.com/oldnewthing/archive/2004/03/29/101121.aspx for
///  // more information.
///  if (IsCtrlDown())
///    shortcut = ApplyModifierToAcceleratorString(shortcut, IDS_APP_CTRL_KEY);
///  else if (IsAltDown())
///    shortcut = ApplyModifierToAcceleratorString(shortcut, IDS_APP_ALT_KEY);
///
///  if (IsCmdDown()) {
///#if defined(MINI_CHROMIUM_OS_MACOSX)
///    shortcut = ApplyModifierToAcceleratorString(shortcut, IDS_APP_COMMAND_KEY);
///#elif defined(MINI_CHROMIUM_OS_WIN)
///    shortcut = ApplyModifierToAcceleratorString(shortcut, IDS_APP_WINDOWS_KEY);
///#else
///    CR_NOTREACHED();
///#endif
///  }

  return shortcut;
}

cr::string16 Accelerator::ApplyShortFormModifiers(
    cr::string16 shortcut) const {
  const cr::char16 kCommandSymbol[] = {0x2318, 0};
  const cr::char16 kCtrlSymbol[] = {0x2303, 0};
  const cr::char16 kShiftSymbol[] = {0x21e7, 0};
  const cr::char16 kOptionSymbol[] = {0x2325, 0};
  const cr::char16 kNoSymbol[] = {0};

  std::vector<cr::string16> parts;
  parts.push_back(cr::string16(IsCtrlDown() ? kCtrlSymbol : kNoSymbol));
  parts.push_back(cr::string16(IsAltDown() ? kOptionSymbol : kNoSymbol));
  parts.push_back(cr::string16(IsShiftDown() ? kShiftSymbol : kNoSymbol));
  parts.push_back(cr::string16(IsCmdDown() ? kCommandSymbol : kNoSymbol));
  parts.push_back(shortcut);
  return cr::StrCat(parts);
}

}  // namespace crui

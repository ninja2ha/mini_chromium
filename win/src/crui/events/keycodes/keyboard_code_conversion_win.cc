// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/keycodes/keyboard_code_conversion_win.h"
#include "crui/events/keycodes/dom/keycode_converter.h"

namespace crui {

namespace {

const WORD kAltPrintScreenScanCode = 0x0054;
const WORD kPrintScreenScanCode = 0xe037;
const WORD kCtrlPauseScanCode = 0xe046;
const WORD kPauseScanCode = 0x0045;
const WORD kHanjaKeyKoreanLayoutScanCode = 0xe0f1;
const WORD kHanjaKeyScanCode = 0x0071;
const WORD kHanYeongKeyKoreanLayoutScanCode = 0xe0f2;
const WORD kHanYeongKeyScanCode = 0x0072;

}  // namespace

WORD WindowsKeyCodeForKeyboardCode(KeyboardCode keycode) {
  return static_cast<WORD>(keycode);
}

KeyboardCode KeyboardCodeForWindowsKeyCode(WORD keycode) {
  return static_cast<KeyboardCode>(keycode);
}

DomCode CodeForWindowsScanCode(WORD scan_code) {
  // There are a few instances where multiple scancodes map to the same
  // physical key. Instead of teaching the NativeKeyCode table to support
  // multiple values; we do this simple translation before calling it.
  switch (scan_code) {
    case kAltPrintScreenScanCode:
      scan_code = kPrintScreenScanCode;
      break;
    case kCtrlPauseScanCode:
      scan_code = kPauseScanCode;
      break;
    case kHanjaKeyKoreanLayoutScanCode:
      scan_code = kHanjaKeyScanCode;
      break;
    case kHanYeongKeyKoreanLayoutScanCode:
      scan_code = kHanYeongKeyScanCode;
      break;
  }

  return crui::KeycodeConverter::NativeKeycodeToDomCode(scan_code);
}

}  // namespace crui

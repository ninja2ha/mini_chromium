// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/win/message_box_win.h"

///#include "crbase/i18n/rtl.h"

namespace crui {

// In addition to passing the RTL flags to ::MessageBox if we are running in an
// RTL locale, we need to make sure that LTR strings are rendered correctly by
// adding the appropriate Unicode directionality marks.
int MessageBox(HWND hwnd,
               const cr::string16& text,
               const cr::string16& caption,
               UINT flags) {
  UINT actual_flags = flags;
  ///if (cr::i18n::IsRTL())
  ///  actual_flags |= MB_RIGHT | MB_RTLREADING;

  cr::string16 localized_text = text;
  ///base::i18n::AdjustStringForLocaleDirection(&localized_text);
  const wchar_t* text_ptr = localized_text.c_str();

  cr::string16 localized_caption = caption;
  ///base::i18n::AdjustStringForLocaleDirection(&localized_caption);
  const wchar_t* caption_ptr = localized_caption.c_str();

  return ::MessageBoxW(hwnd, text_ptr, caption_ptr, actual_flags);
}

}  // namespace crui

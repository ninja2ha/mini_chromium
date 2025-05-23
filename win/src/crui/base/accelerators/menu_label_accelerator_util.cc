// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/accelerators/menu_label_accelerator_util.h"

///#include "crbase/i18n/case_conversion.h"
#include "crbase/strings/string_util.h"

namespace crui {

cr::char16 GetMnemonic(const cr::string16& label) {
  ///size_t index = 0;
  ///do {
  ///  index = label.find('&', index);
  ///  if (index != cr::string16::npos) {
  ///    if (index + 1 != label.size()) {
  ///      if (label[index + 1] != '&') {
  ///        cr::char16 char_array[] = {label[index + 1], 0};
  ///        // TODO(jshin): What about Turkish locale? See http://crbug.com/81719.
  ///        // If the mnemonic is capital I and the UI language is Turkish,
  ///        // lowercasing it results in 'small dotless i', which is different
  ///        // from a 'dotted i'. Similar issues may exist for az and lt locales.
  ///        return cr::i18n::ToLower(char_array)[0];
  ///      } else {
  ///        index++;
  ///      }
  ///    }
  ///    index++;
  ///  }
  ///} while (index != cr::string16::npos);
  return 0;
}

cr::string16 EscapeMenuLabelAmpersands(const cr::string16& label) {
  cr::string16 ret;
  static const cr::char16 kAmps[] = {'&', 0};
  static const cr::char16 kTwoAmps[] = {'&', '&', 0};
  cr::ReplaceChars(label, kAmps, kTwoAmps, &ret);
  return ret;
}

}  // namespace crui

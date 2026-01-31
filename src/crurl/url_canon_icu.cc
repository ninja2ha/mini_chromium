// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crurl/url_canon.h"

#include <vector>

#include "crbase/logging/logging.h"
#include "crbase/strings/string_split.h"

#include "crurl/url_canon_internal.h"
#include "crurl/punycode.h"

namespace crurl {

bool IDNToASCII(const char16_t* src, int src_len, CanonOutputW* output) {
  CR_DCHECK(output->length() == 0);  // Output buffer is assumed empty.

  cr::StringPiece16 input(src, src_len);
  constexpr cr::StringPiece16 separators(u".");
  std::vector<cr::StringPiece16> members = cr::SplitStringPiece(
      input, separators, cr::TRIM_WHITESPACE, cr::SPLIT_WANT_ALL);

  std::u16string temp;
  for (size_t i = 0; i < members.size(); i++) {
    if (!EncodePunycode(members[i], temp))
      return false;

    if (i) output->Append(u".", 1);

    if (temp.length() == members[i].length() + 1) {
      // unsafe_cast
      output->Append(members[i].begin(), static_cast<int>(members[i].length()));
    } else if (!temp.empty()) {
      output->Append(u"xn--", 4);
      // unsafe_cast
      output->Append(temp.c_str(), static_cast<int>(temp.length()));
    }
  }

  return true;
}

}  // namespace crurl
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crurl/url_canon.h"

#include <vector>

#include "crurl/url_canon_internal.h"

#include "crbase/endecoding/punycode.h"
#include "crbase/logging.h"
#include "crbase/strings/string_split.h"

namespace crurl {

bool IDNToASCII(const cr::char16* src, int src_len, CanonOutputW* output) {
  CR_DCHECK(output->length() == 0);  // Output buffer is assumed empty.

  cr::StringPiece16 input(src, src_len);
  cr::StringPiece16 separators(L".");
  std::vector<cr::StringPiece16> members = cr::SplitStringPiece(
      input, separators, cr::TRIM_WHITESPACE, cr::SPLIT_WANT_ALL);

  std::wstring temp;
  for (size_t i = 0; i < members.size(); i++) {
    if (!cr::EncodePunycode(members[i], temp))
      return false;

    if (i) output->Append(L".", 1);

    if (temp.length() == members[i].length() + 1) {
      output->Append(members[i].begin(), static_cast<int>(members[i].length()));
    } else if (!temp.empty()) {
      output->Append(L"xn--", 4);
      output->Append(temp.c_str(), static_cast<int>(temp.length()));
    }
  }

  return true;
}

}  // namespace crurl

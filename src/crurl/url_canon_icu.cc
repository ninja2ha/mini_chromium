// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crurl/url_canon.h"

#include <vector>

#include "crbase/logging/logging.h"
#include "crbase/strings/string_split.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/utf_string_conversions.h"

#include "crurl/url_canon_internal.h"
#include "crurl/punycode.h"

namespace crurl {

bool IDNToASCII(const char16_t* src, int src_len, CanonOutputW* output) {
  CR_DCHECK(output->length() == 0);  // Output buffer is assumed empty.

  constexpr cr::StringPiece16 punycode_prefix(u"xn--");
  constexpr cr::StringPiece16 dot(u".");

  std::u16string input_lower(src, src_len);
  std::for_each(input_lower.begin(), input_lower.end(), [](char16_t& c) {
    // U+FE6A which is a type of percent
    if (c == 0xFE6A) {
      c = '%';
      return ;
    }

    c = cr::ToLower(c);
  });

  const cr::StringPiece16 input(input_lower);
  std::vector<cr::StringPiece16> results = 
      cr::SplitStringPiece(input, dot, cr::KEEP_WHITESPACE, cr::SPLIT_WANT_ALL);

  std::u16string u16out, punycode;
  for(size_t i = 0; i < results.size(); i++) {
    if (i)
      u16out.push_back(u'.');

    const cr::StringPiece16& item = results[i];
    if (cr::IsStringASCII(item)) {
      u16out.append(item.data(), static_cast<int>(item.length()));
      continue;
    }

    if (cr::StartsWith(item, punycode_prefix, 
                       cr::CompareCase::INSENSITIVE_ASCII)) {
      return false;
    }

    punycode.clear();
    if (!EncodePunycode(item, punycode))
      return false;

    if (punycode.length() == item.length() + 1) {
      u16out.append(punycode.c_str(),
                    static_cast<int>(punycode.length() - 1));
    } else if (!punycode.empty()) {
      u16out.append(punycode_prefix.data(),
                    static_cast<int>(punycode_prefix.length()));
      // unsafe_cast
      u16out.append(punycode.c_str(),
                    static_cast<int>(punycode.length()));
    }
  }

  output->Append(u16out.data(), static_cast<int>(u16out.length()));
  output->push_back(u'\x0000');
  output->set_length(output->length() - 1);
  return true;
}

}  // namespace crurl
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crurl/url_canon.h"

#include <vector>

#include "crbase/logging/logging.h"
#include "crbase/strings/string_split.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/numerics/safe_conversions.h"

#include "crurl/url_canon_internal.h"

namespace crurl {

bool IDNToASCII(const char16_t* src, int src_len, CanonOutput* output) {
  CR_DCHECK(output->length() == 0);  // Output buffer is assumed empty.

  cr::StringPiece16 utf16(src, src_len);
  if (!cr::IsStringASCII(utf16)) {
    CR_DLOG(Warning) << "ascii domain support only!";
    return true;
  }

  std::string out = cr::UTF16ToASCII(utf16);
  int length = cr::checked_cast<int>(out.length());
  output->Append(out.data(), length);
  output->Resize(length);
  return true;
}

}  // namespace crurl
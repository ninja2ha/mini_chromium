// Copyright 2016 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRURL_PUNYCODE_H_
#define MINI_CHROMIUM_SRC_CRURL_PUNYCODE_H_

#include <string>

#include "crbase/strings/string_piece.h"

#include "crurl/url_export.h"

namespace crurl {

// Encodes an UTF16 string into Punycode. Returns true if successful and false
// otherwise.
CRURL_EXPORT bool EncodePunycode(const cr::StringPiece16& input, 
                                 std::string& out);
CRURL_EXPORT bool EncodePunycode(const cr::StringPiece16& input, 
                                 std::u16string& out);

// Decodes a Punycode string into a sequence of Unicode scalars. Return true if
// successful and false otherwise.
CRURL_EXPORT bool DecodePunycode(cr::StringPiece input, std::u16string& output);

}  // namespace crurl

#endif  // MINI_CHROMIUM_SRC_CRURL_PUNYCODE_H_
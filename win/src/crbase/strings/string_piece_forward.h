// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Forward declaration of StringPiece types from crbase/strings/string_piece.h

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_FORWARD_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_FORWARD_H_

#include <string>

#include "crbase/strings/string16.h"

namespace cr {

template <typename STRING_TYPE>
class BasicStringPiece;
typedef BasicStringPiece<std::string> StringPiece;
typedef BasicStringPiece<string16> StringPiece16;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_FORWARD_H_
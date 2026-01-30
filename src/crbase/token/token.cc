// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/token/token.h"

#include <inttypes.h>

#include "crbase/rand_util.h"
#include "crbase/strings/stringprintf.h"

namespace cr {

// static
Token Token::CreateRandom() {
  Token token;

  // Use cr::RandBytes instead of crypto::RandBytes, because crypto calls the
  // base version directly, and to prevent the dependency from base/ to crypto/.
  cr::RandBytes(&token, sizeof(token));
  return token;
}

std::string Token::ToString() const {
  return cr::StringPrintf("%016" PRIX64 "%016" PRIX64, high_, low_);
}

}  // namespace cr

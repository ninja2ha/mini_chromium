// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "crbase/stl_util.h"

#include "gtest/gtest.h"

#include "crbase/strings/utf_string_conversions.h"
#include "crurl/punycode.h"


namespace crurl {

TEST(URLPunycodeTest, EncodePunycode) {
  struct EncodeCase {
    const char* input;
    const char* expected;
    bool expected_success;
  } punycode_cases[] = {
    {"M\xc3\x9cNCHEN", "mnchen-3ya", true}
  };

  for (const auto & i : punycode_cases) {
    std::string out;
    bool success = crurl::EncodePunycode(cr::UTF8ToUTF16(i.input), out);
    EXPECT_EQ(success, i.expected_success);
    EXPECT_EQ(out, i.expected);
  }
}

}  // namespace crurl

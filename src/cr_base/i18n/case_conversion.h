// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_I18N_CASE_CONVERSION_H_
#define MINI_CHROMIUM_SRC_CRBASE_I18N_CASE_CONVERSION_H_

#include <string>

#include "cr_base/base_export.h"
#include "cr_base/strings/string_piece.h"

namespace cr {
namespace i18n {

// UNICODE CASE-HANDLING ADVICE
//
// In English it's always safe to convert to upper-case or lower-case text
// and get a good answer. But some languages have rules specific to those
// locales. One example is the Turkish I:
//   http://www.i18nguy.com/unicode/turkish-i18n.html
//
// Note that case conversions will change the length of the string in some
// not-uncommon cases. Never assume that the output is the same length as
// the input.

// Returns the lower case equivalent of char.
int32_t ToLower(int32_t c);

}  // namespace i18n
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_I18N_CASE_CONVERSION_H_
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/strings/strcat.h"

#include <string>

#include "crbase/strings/strcat_internal.h"

namespace cr {

std::string StrCat(Span<const StringPiece> pieces) {
  return internal::StrCatT(pieces);
}

std::u16string StrCat(Span<const StringPiece16> pieces) {
  return internal::StrCatT(pieces);
}

std::wstring StrCat(Span<const WStringPiece> pieces) {
  return internal::StrCatT(pieces);
}

std::string StrCat(Span<const std::string> pieces) {
  return internal::StrCatT(pieces);
}

std::u16string StrCat(Span<const std::u16string> pieces) {
  return internal::StrCatT(pieces);
}

std::wstring StrCat(Span<const std::wstring> pieces) {
  return internal::StrCatT(pieces);
}

void StrAppend(std::string* dest, Span<const StringPiece> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::u16string* dest, Span<const StringPiece16> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::wstring* dest, Span<const WStringPiece> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::string* dest, Span<const std::string> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::u16string* dest, Span<const std::u16string> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::wstring* dest, Span<const std::wstring> pieces) {
  internal::StrAppendT(*dest, pieces);
}

}  // namespace cr

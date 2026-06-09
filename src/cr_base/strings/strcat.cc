// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/strings/strcat.h"

#include <string>

#include "cr_base/strings/internal/strcat_internal.h"

namespace cr {

// StrCat ----------------------------------------------------------------------

std::string StrCat(Span<const StringPiece> pieces) {
  return internal::StrCatT(pieces);
}

std::u16string StrCat(Span<const StringPiece16> pieces) {
  return internal::StrCatT(pieces);
}

std::u32string StrCat(Span<const StringPiece32> pieces) {
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

std::u32string StrCat(Span<const std::u32string> pieces) {
  return internal::StrCatT(pieces);
}

std::wstring StrCat(Span<const std::wstring> pieces) {
  return internal::StrCatT(pieces);
}

// StrAppend -------------------------------------------------------------------

void StrAppend(std::string* dest, Span<const StringPiece> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::u16string* dest, Span<const StringPiece16> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::u32string* dest, Span<const StringPiece32> pieces) {
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

void StrAppend(std::u32string* dest, Span<const std::u32string> pieces) {
  internal::StrAppendT(*dest, pieces);
}

void StrAppend(std::wstring* dest, Span<const std::wstring> pieces) {
  internal::StrAppendT(*dest, pieces);
}

// JoinString ------------------------------------------------------------------

std::string JoinString(Span<const std::string> parts, 
                       StringPiece separator) {
  return internal::JoinStringT(parts, separator);
}

std::u16string JoinString(Span<const std::u16string> parts,
                          StringPiece16 separator) {
  return internal::JoinStringT(parts, separator);
}

std::u32string JoinString(Span<const std::u32string> parts,
                          StringPiece32 separator) {
  return internal::JoinStringT(parts, separator);
}

std::wstring JoinString(Span<const std::wstring> parts,
                        WStringPiece separator) {
  return internal::JoinStringT(parts, separator);
}

std::string JoinString(Span<const StringPiece> parts, 
                       StringPiece separator) {
  return internal::JoinStringT(parts, separator);
}

std::u16string JoinString(Span<const StringPiece16> parts,
                          StringPiece16 separator) {
  return internal::JoinStringT(parts, separator);
}

std::u32string JoinString(Span<const StringPiece32> parts,
                          StringPiece32 separator) {
  return internal::JoinStringT(parts, separator);
}

std::wstring JoinString(Span<const WStringPiece> parts,
                        WStringPiece separator) {
  return internal::JoinStringT(parts, separator);
}


}  // namespace cr
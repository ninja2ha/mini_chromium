// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRCAT_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRCAT_H_

#include <initializer_list>

#include "crbase/base_export.h"
#include "crbase/containers/span.h"
#include "crbase/strings/string_piece.h"
#include "crbuild/compiler_specific.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
// Guard against conflict with Win32 API StrCat macro:
// check StrCat wasn't and will not be redefined.
#define StrCat StrCat
#endif

namespace cr {

// StrCat ----------------------------------------------------------------------
//
// StrCat is a function to perform concatenation on a sequence of strings.
// It is preferrable to a sequence of "a + b + c" because it is both faster and
// generates less code.
//
//   std::string result = cr::StrCat({"foo ", result, "\nfoo ", bar});
//
// To join an array of strings with a separator, see cr::JoinString in
// base/strings/string_util.h.
//
// MORE INFO
//
// StrCat can see all arguments at once, so it can allocate one return buffer
// of exactly the right size and copy once, as opposed to a sequence of
// operator+ which generates a series of temporary strings, copying as it goes.
// And by using StringPiece arguments, StrCat can avoid creating temporary
// string objects for char* constants.
//
// ALTERNATIVES
//
// Internal Google / Abseil has a similar StrCat function. That version takes
// an overloaded number of arguments instead of initializer list (overflowing
// to initializer list for many arguments). We don't have any legacy
// requirements and using only initializer_list is simpler and generates
// roughly the same amount of code at the call sites.
//
// Abseil's StrCat also allows numbers by using an intermediate class that can
// be implicitly constructed from either a string or various number types. This
// class formats the numbers into a static buffer for increased performance,
// and the call sites look nice.
//
// As-written Abseil's helper class for numbers generates slightly more code
// than the raw StringPiece version. We can de-inline the helper class'
// constructors which will cause the StringPiece constructors to be de-inlined
// for this call and generate slightly less code. This is something we can
// explore more in the future.

CRBASE_EXPORT std::string StrCat(Span<const StringPiece> pieces)
    CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::u16string StrCat(Span<const StringPiece16> pieces)
    CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::wstring StrCat(Span<const WStringPiece> pieces)
    CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::string StrCat(Span<const std::string> pieces)
    CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::u16string StrCat(Span<const std::u16string> pieces)
    CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::wstring StrCat(Span<const std::wstring> pieces)
    CR_WARN_UNUSED_RESULT;

// Initializer list forwards to the array version.
inline std::string StrCat(std::initializer_list<StringPiece> pieces) {
  return StrCat(MakeSpan(pieces));
}

inline std::u16string StrCat(std::initializer_list<StringPiece16> pieces) {
  return StrCat(MakeSpan(pieces));
}

inline std::wstring StrCat(std::initializer_list<WStringPiece> pieces) {
  return StrCat(MakeSpan(pieces));
}

// StrAppend -------------------------------------------------------------------
//
// Appends a sequence of strings to a destination. Prefer:
//   StrAppend(&foo, ...);
// over:
//   foo += StrCat(...);
// because it avoids a temporary string allocation and copy.

CRBASE_EXPORT void StrAppend(std::string* dest, Span<const StringPiece> pieces);
CRBASE_EXPORT void StrAppend(std::u16string* dest,
                             Span<const StringPiece16> pieces);
CRBASE_EXPORT void StrAppend(std::wstring* dest,
                             Span<const WStringPiece> pieces);
CRBASE_EXPORT void StrAppend(std::string* dest, Span<const std::string> pieces);
CRBASE_EXPORT void StrAppend(std::u16string* dest,
                             Span<const std::u16string> pieces);
CRBASE_EXPORT void StrAppend(std::wstring* dest,
                             Span<const std::wstring> pieces);

// Initializer list forwards to the array version.
inline void StrAppend(std::string* dest,
                      std::initializer_list<StringPiece> pieces) {
  StrAppend(dest, MakeSpan(pieces));
}

inline void StrAppend(std::u16string* dest,
                      std::initializer_list<StringPiece16> pieces) {
  StrAppend(dest, MakeSpan(pieces));
}

inline void StrAppend(std::wstring* dest,
                      std::initializer_list<WStringPiece> pieces) {
  StrAppend(dest, MakeSpan(pieces));
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRCAT_H_

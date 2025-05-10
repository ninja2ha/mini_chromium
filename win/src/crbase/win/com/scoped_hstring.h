// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_COM_SCOPED_HSTRING_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_COM_SCOPED_HSTRING_H_

#include <hstring.h>

#include "crbase/helper/scoped_generic.h"
#include "crbase/strings/string_piece_forward.h"

namespace cr {

namespace internal {

// Scoped HSTRING class to maintain lifetime of HSTRINGs allocated with
// WindowsCreateString().
struct CRBASE_EXPORT ScopedHStringTraits {
  static HSTRING InvalidValue() { return nullptr; }
  static void Free(HSTRING hstr);
};

}  // namespace internal

namespace win {

// ScopedHString is a wrapper around an HSTRING. Note that it requires certain
// functions that are only available on Windows 8 and later, and that these
// functions need to be delayloaded to avoid breaking Chrome on Windows 7.
//
// Callers MUST check the return value of ResolveCoreWinRTStringDelayLoad()
// *before* using ScopedHString.
//
// One-time Initialization for ScopedHString:
//
//   bool success = ScopedHString::ResolveCoreWinRTStringDelayload();
//   if (success) {
//     // ScopeHString can be used.
//   } else {
//     // Handle error.
//   }
//
// Example use:
//
//   ScopedHString string = ScopedHString::Create(L"abc");
//
// Also:
//
//   HSTRING win_string;
//   HRESULT hr = WindowsCreateString(..., &win_string);
//   ScopedHString string(win_string);
//
class CRBASE_EXPORT ScopedHString
    : public ScopedGeneric<HSTRING, cr::internal::ScopedHStringTraits> {
 public:
  // Constructs a ScopedHString from an HSTRING, and takes ownership of |hstr|.
  explicit ScopedHString(HSTRING hstr);

  static ScopedHString Create(StringPiece16 str);
  static ScopedHString Create(StringPiece str);

  // Loads all required HSTRING functions, available from Win8 and onwards.
  static bool ResolveCoreWinRTStringDelayload();

  // Returns a view into the memory buffer managed by the instance. The returned
  // StringPiece is only valid during the lifetime of this ScopedHString
  // instance.
  StringPiece16 Get() const;

  // Returns a copy of the instance as a UTF-8 string.
  std::string GetAsUTF8() const;
};

}  // namespace win
}  // namespace cr

#endif  // BASE_WIN_SCOPED_HSTRING_H_

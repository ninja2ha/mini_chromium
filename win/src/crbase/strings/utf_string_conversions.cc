// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/strings/utf_string_conversions.h"

#include <stdint.h>

#include "crbase/strings/string_piece.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/utf_string_conversion_utils.h"
#include "crbase/compiler_specific.h"
#include "crbase/build_platform.h"

namespace cr {

namespace {

// Generalized Unicode converter -----------------------------------------------

// Converts the given source Unicode character type to the given destination
// Unicode character type as a STL string. The given input buffer and size
// determine the source, and the given output STL string will be replaced by
// the result.
template<typename SRC_CHAR, typename DEST_STRING>
bool ConvertUnicode(const SRC_CHAR* src,
                    size_t src_len,
                    DEST_STRING* output) {
  // ICU requires 32-bit numbers.
  bool success = true;
  int32_t src_len32 = static_cast<int32_t>(src_len);
  for (int32_t i = 0; i < src_len32; i++) {
    uint32_t code_point;
    if (ReadUnicodeCharacter(src, src_len32, &i, &code_point)) {
      WriteUnicodeCharacter(code_point, output);
    } else {
      WriteUnicodeCharacter(0xFFFD, output);
      success = false;
    }
  }

  return success;
}

template<typename STR1, typename STR2>
void CopyASCIIT(const STR1 src, size_t src_len, STR2* out) {
  if (!src_len)
    return;

  out->resize(src_len);
  for (size_t i = 0; i < src_len; i++) 
    (*out)[i] = static_cast<STR2::value_type>(src[i]);
}

}  // namespace

// UTF-8 <-> Wide --------------------------------------------------------------

bool WideToUTF8(const wchar_t* src, size_t src_len, std::string* output) {
  if (IsStringASCII(std::wstring(src, src_len))) {
    CopyASCIIT(src, src_len, output);
    return true;
  } else {
    PrepareForUTF8Output(src, src_len, output);
    return ConvertUnicode(src, src_len, output);
  }
}

std::string WideToUTF8(const std::wstring& wide) {
  std::string ret;
  if (IsStringASCII(wide)) { 
    CopyASCIIT(wide.c_str(), wide.length(), &ret);
    return ret;
  }

  PrepareForUTF8Output(wide.data(), wide.length(), &ret);
  ConvertUnicode(wide.data(), wide.length(), &ret);
  return ret;
}

bool UTF8ToWide(const char* src, size_t src_len, std::wstring* output) {
  if (IsStringASCII(StringPiece(src, src_len))) {
    CopyASCIIT(src, src_len, output);
    return true;
  } else {
    PrepareForUTF16Or32Output(src, src_len, output);
    return ConvertUnicode(src, src_len, output);
  }
}

std::wstring UTF8ToWide(StringPiece utf8) {
  std::wstring ret;
  if (IsStringASCII(utf8)) {
    CopyASCIIT(utf8.data(), utf8.length(), &ret);
    return ret;
  }

  PrepareForUTF16Or32Output(utf8.data(), utf8.length(), &ret);
  ConvertUnicode(utf8.data(), utf8.length(), &ret);
  return ret;
}

// UTF-16 <-> Wide -------------------------------------------------------------

#if defined(CR_WCHAR_T_IS_UTF16)

// When wide == UTF-16, then conversions are a NOP.
bool WideToUTF16(const wchar_t* src, size_t src_len, string16* output) {
  output->assign(src, src_len);
  return true;
}

string16 WideToUTF16(const std::wstring& wide) {
  return wide;
}

bool UTF16ToWide(const char16* src, size_t src_len, std::wstring* output) {
  output->assign(src, src_len);
  return true;
}

std::wstring UTF16ToWide(const string16& utf16) {
  return utf16;
}

#elif defined(CR_WCHAR_T_IS_UTF32)

bool WideToUTF16(const wchar_t* src, size_t src_len, string16* output) {
  output->clear();
  // Assume that normally we won't have any non-BMP characters so the counts
  // will be the same.
  output->reserve(src_len);
  return ConvertUnicode(src, src_len, output);
}

string16 WideToUTF16(const std::wstring& wide) {
  string16 ret;
  WideToUTF16(wide.data(), wide.length(), &ret);
  return ret;
}

bool UTF16ToWide(const char16* src, size_t src_len, std::wstring* output) {
  output->clear();
  // Assume that normally we won't have any non-BMP characters so the counts
  // will be the same.
  output->reserve(src_len);
  return ConvertUnicode(src, src_len, output);
}

std::wstring UTF16ToWide(const string16& utf16) {
  std::wstring ret;
  UTF16ToWide(utf16.data(), utf16.length(), &ret);
  return ret;
}

#endif  // defined(CR_WCHAR_T_IS_UTF32)

// UTF16 <-> UTF8 --------------------------------------------------------------

#if defined(CR_WCHAR_T_IS_UTF32)

bool UTF8ToUTF16(const char* src, size_t src_len, string16* output) {
  if (IsStringASCII(StringPiece(src, src_len))) {
    output->assign(src, src + src_len);
    return true;
  } else {
    PrepareForUTF16Or32Output(src, src_len, output);
    return ConvertUnicode(src, src_len, output);
  }
}

string16 UTF8ToUTF16(StringPiece utf8) {
  if (IsStringASCII(utf8)) {
    return string16(utf8.begin(), utf8.end());
  }

  string16 ret;
  PrepareForUTF16Or32Output(utf8.data(), utf8.length(), &ret);
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  ConvertUnicode(utf8.data(), utf8.length(), &ret);
  return ret;
}

bool UTF16ToUTF8(const char16* src, size_t src_len, std::string* output) {
  if (IsStringASCII(StringPiece16(src, src_len))) {
    output->assign(src, src + src_len);
    return true;
  } else {
    PrepareForUTF8Output(src, src_len, output);
    return ConvertUnicode(src, src_len, output);
  }
}

std::string UTF16ToUTF8(StringPiece16 utf16) {
  std::string ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF16ToUTF8(utf16.data(), utf16.length(), &ret);
  return ret;
}

#elif defined(CR_WCHAR_T_IS_UTF16)
// Easy case since we can use the "wide" versions we already wrote above.

bool UTF8ToUTF16(const char* src, size_t src_len, string16* output) {
  return UTF8ToWide(src, src_len, output);
}

string16 UTF8ToUTF16(StringPiece utf8) {
  return UTF8ToWide(utf8);
}

bool UTF16ToUTF8(const char16* src, size_t src_len, std::string* output) {
  return WideToUTF8(src, src_len, output);
}

std::string UTF16ToUTF8(StringPiece16 utf16) {
  std::string ret;
  if (IsStringASCII(utf16)) {
    CopyASCIIT(utf16.data(), utf16.length(), &ret);
    return ret;
  }

  PrepareForUTF8Output(utf16.data(), utf16.length(), &ret);
  ConvertUnicode(utf16.data(), utf16.length(), &ret);
  return ret;
}

#endif

string16 ASCIIToUTF16(StringPiece ascii) {
  CR_DCHECK(IsStringASCII(ascii)) << ascii;
  return string16(ascii.begin(), ascii.end());
}

std::string UTF16ToASCII(StringPiece16 utf16) {
  CR_DCHECK(IsStringASCII(utf16)) << UTF16ToUTF8(utf16);
  std::string ret;
  CopyASCIIT(utf16.data(), utf16.length(), &ret);
  return ret;
}

}  // namespace cr
// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/strings/utf_string_conversions.h"

#include <limits.h>
#include <stdint.h>

#include <type_traits>

#include "crbase/logging/logging.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/utf_string_conversion_utils.h"
#include "crbase/third_party/icu/icu_utf.h"
#include "crbuild/build_config.h"

namespace cr {

namespace {

constexpr int32_t kErrorCodePoint = 0xFFFD;

// Size coefficient ----------------------------------------------------------
// The maximum number of codeunits in the destination encoding corresponding to
// one codeunit in the source encoding.

template <typename SrcChar, typename DestChar>
struct SizeCoefficient {
  static_assert(sizeof(SrcChar) < sizeof(DestChar),
                "Default case: from a smaller encoding to the bigger one");

  // ASCII symbols are encoded by one codeunit in all encodings.
  static constexpr int value = 1;
};

template <>
struct SizeCoefficient<char16_t, char> {
  // One UTF-16 codeunit corresponds to at most 3 codeunits in UTF-8.
  static constexpr int value = 3;
};

template <>
struct SizeCoefficient<char32_t, char> {
  // UTF-8 uses at most 4 codeunits per character.
  static constexpr int value = 4;
};

template <>
struct SizeCoefficient<char32_t, char16_t> {
  // UTF-32 uses at most 2 codeunits per character.
  static constexpr int value = 2;
};

template <>
struct SizeCoefficient<char32_t, wchar_t> {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  // UTF-32 uses at most 2 codeunits per character.
  static constexpr int value = 2;
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  static constexpr int value = 1;
#endif
};

template <>
struct SizeCoefficient<wchar_t, char> {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  // One wchar_t codeunit corresponds to at most 3 codeunits in UTF-8.
  static constexpr int value = 3;
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  // UTF-8 uses at most 4 codeunits per character.
  static constexpr int value = 4;
#endif
};

template <>
struct SizeCoefficient<wchar_t, char16_t> {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  static constexpr int value = 1;
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  // UTF-32 uses at most 2 codeunits per character.
  static constexpr int value = 2;
#endif
};

template <typename SrcChar, typename DestChar>
constexpr int size_coefficient_v =
    SizeCoefficient<std::decay_t<SrcChar>, std::decay_t<DestChar>>::value;

// UnicodeAppendUnsafe --------------------------------------------------------
// Function overloads that write code_point to the output string. Output string
// has to have enough space for the codepoint.

// Convenience typedef that checks whether the passed in type is integral (i.e.
// bool, char, int or their extended versions) and is of the correct size.
template <typename Char, size_t N>
using EnableIfBitsAre = std::enable_if_t<std::is_integral<Char>::value &&
                                             CHAR_BIT * sizeof(Char) == N,
                                         bool>;

template <typename Char, EnableIfBitsAre<Char, 8> = true>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  CBU8_APPEND_UNSAFE(out, *size, code_point);
}

template <typename Char, EnableIfBitsAre<Char, 16> = true>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  CBU16_APPEND_UNSAFE(out, *size, code_point);
}

template <typename Char, EnableIfBitsAre<Char, 32> = true>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  out[(*size)++] = code_point;
}

// DoUTFConversion ------------------------------------------------------------
// Main driver of UTFConversion specialized for different Src encodings.
// dest has to have enough room for the converted text.

template <typename DestChar>
bool DoUTFConversion(const char* src,
                     int32_t src_len,
                     DestChar* dest,
                     int32_t* dest_len) {
  bool success = true;

  for (int32_t i = 0; i < src_len;) {
    int32_t code_point;
    CBU8_NEXT(src, i, src_len, code_point);

    if (!IsValidCodepoint(code_point)) {
      success = false;
      code_point = kErrorCodePoint;
    }

    UnicodeAppendUnsafe(dest, dest_len, code_point);
  }

  return success;
}

template <typename DestChar>
bool DoUTFConversion(const char16_t* src,
                     int32_t src_len,
                     DestChar* dest,
                     int32_t* dest_len) {
  bool success = true;

  auto ConvertSingleChar = [&success](char16_t in) -> int32_t {
    if (!CBU16_IS_SINGLE(in) || !IsValidCodepoint(in)) {
      success = false;
      return kErrorCodePoint;
    }
    return in;
  };

  int32_t i = 0;

  // Always have another symbol in order to avoid checking boundaries in the
  // middle of the surrogate pair.
  while (i < src_len - 1) {
    int32_t code_point;

    if (CBU16_IS_LEAD(src[i]) && CBU16_IS_TRAIL(src[i + 1])) {
      code_point = CBU16_GET_SUPPLEMENTARY(src[i], src[i + 1]);
      if (!IsValidCodepoint(code_point)) {
        code_point = kErrorCodePoint;
        success = false;
      }
      i += 2;
    } else {
      code_point = ConvertSingleChar(src[i]);
      ++i;
    }

    UnicodeAppendUnsafe(dest, dest_len, code_point);
  }

  if (i < src_len)
    UnicodeAppendUnsafe(dest, dest_len, ConvertSingleChar(src[i]));

  return success;
}

template <typename DestChar>
bool DoUTFConversion(const char32_t* src,
                     int32_t src_len,
                     DestChar* dest,
                     int32_t* dest_len) {
  bool success = true;

  for (int32_t i = 0; i < src_len; ++i) {
    int32_t code_point = src[i];

    if (!IsValidCodepoint(code_point)) {
      success = false;
      code_point = kErrorCodePoint;
    }

    UnicodeAppendUnsafe(dest, dest_len, code_point);
  }

  return success;
}

// UTFConversion --------------------------------------------------------------
// Function template for generating all UTF conversions.

template <typename InputString, typename DestString>
bool 
UTFConversion(const InputString& src_str, DestString* dest_str) {
  if (IsStringASCII(src_str)) {
    dest_str->assign(src_str.begin(), src_str.end());
    return true;
  }

  dest_str->resize(src_str.length() *
                   size_coefficient_v<typename InputString::value_type,
                                      typename DestString::value_type>);

  // Empty string is ASCII => it OK to call operator[].
  auto* dest = &(*dest_str)[0];

  // ICU requires 32 bit numbers.
  int32_t src_len32 = static_cast<int32_t>(src_str.length());
  int32_t dest_len32 = 0;

  bool res = DoUTFConversion(src_str.data(), src_len32, dest, &dest_len32);

  dest_str->resize(dest_len32);
  dest_str->shrink_to_fit();
  return res;
}

}  // namespace

// UTF8 -> UTF16 --------------------------------------------------------------
bool UTF8ToUTF16(const char* src, size_t src_len, std::u16string* output) {
  return UTFConversion(StringPiece(src, src_len), output);
}

std::u16string UTF8ToUTF16(StringPiece utf8) {
  std::u16string ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF8ToUTF16(utf8.data(), utf8.size(), &ret);
  return ret;
}

// UTF8 -> UTF32 --------------------------------------------------------------
bool UTF8ToUTF32(const char* src, size_t src_len, std::u32string* output) {
  return UTFConversion(StringPiece(src, src_len), output);
}

std::u32string UTF8ToUTF32(StringPiece utf8) {
  std::u32string ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF8ToUTF32(utf8.data(), utf8.size(), &ret);
  return ret;
}

// UTF16 -> UTF8 ---------------------------------------------------------------
bool UTF16ToUTF8(const char16_t* src, size_t src_len, std::string* output) {
  return UTFConversion(StringPiece16(src, src_len), output);
}

std::string UTF16ToUTF8(StringPiece16 utf16) {
  std::string ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF16ToUTF8(utf16.data(), utf16.length(), &ret);
  return ret;
}

// UTF-16 -> UTF32 ------------------------------------------------------------
bool UTF16ToUTF32(const char16_t* src, size_t src_len, std::u32string* output) {
  return UTFConversion(StringPiece16(src, src_len), output);
}

std::u32string UTF16ToUTF32(StringPiece16 utf16) {
  std::u32string ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF16ToUTF32(utf16.data(), utf16.length(), &ret);
  return ret;
}

// UTF-32 -> UTF8 -------------------------------------------------------------
bool UTF32ToUTF8(const char32_t* src, size_t src_len, std::string* output) {
  return UTFConversion(StringPiece32(src, src_len), output);
}

std::string UTF32ToUTF8(StringPiece32 utf32) {
  std::string ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF32ToUTF8(utf32.data(), utf32.length(), &ret);
  return ret;
}

// UTF-32 -> UTF16 -------------------------------------------------------------
bool UTF32ToUTF16(const char32_t* src, size_t src_len, std::u16string* output) {
  return UTFConversion(StringPiece32(src, src_len), output);
}

std::u16string UTF32ToUTF16(StringPiece32 utf32) {
  std::u16string ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF32ToUTF16(utf32.data(), utf32.length(), &ret);
  return ret;
}

// UTF8 -> Wide ----------------------------------------------------------------
bool UTF8ToWide(const char* src, size_t src_len, std::wstring* output) {
  return UTFConversion(StringPiece(src, src_len), output);
}

std::wstring UTF8ToWide(StringPiece utf8) {
  std::wstring ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF8ToWide(utf8.data(), utf8.length(), &ret);
  return ret;
}

// UTF16 ->Wide ----------------------------------------------------------------
bool UTF16ToWide(const char16_t* src, size_t src_len, std::wstring* output) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  output->assign(reinterpret_cast<const wchar_t*>(src), src_len);
  return true;
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  return UTFConversion(StringPiece16(src, src_len), output);
#endif
}

std::wstring UTF16ToWide(StringPiece16 utf16) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return std::wstring(utf16.begin(), utf16.end());
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  std::wstring ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF16ToWide(utf16.data(), utf16.length(), &ret);
  return ret;
#endif
}

// UTF32 ->Wide ----------------------------------------------------------------
bool UTF32ToWide(const char32_t* src, size_t src_len, std::wstring* output) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return UTFConversion(StringPiece32(src, src_len), output);
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  output->assign(reinterpret_cast<const wchar_t*>(src), src_len);
  return true;
#endif
}

std::wstring UTF32ToWide(StringPiece32 utf32) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  std::wstring ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF32ToWide(utf32.data(), utf32.length(), &ret);
  return ret;
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  return std::wstring(utf32.begin(), utf32.end());
#endif
}

// Wide -> UTF8 ----------------------------------------------------------------
bool WideToUTF8(const wchar_t* src, size_t src_len, std::string* output) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return UTF16ToUTF8(reinterpret_cast<const char16_t*>(src), src_len, output);
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  return UTF32ToUTF8(reinterpret_cast<const char32_t*>(src), src_len, output);
#endif
}

std::string WideToUTF8(WStringPiece wide) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return UTF16ToUTF8(
      StringPiece16(reinterpret_cast<const char16_t*>(wide.data()), 
                    wide.size()));
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  return UTF32ToUTF8(
      StringPiece32(reinterpret_cast<const char32_t*>(wide.data()), 
                    wide.size()));
#endif
}

// Wide -> UTF16 ---------------------------------------------------------------
bool WideToUTF16(const wchar_t* src, size_t src_len, std::u16string* output) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  output->assign(reinterpret_cast<const char16_t*>(src), src_len);
  return true;
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  return UTF32ToUTF16(reinterpret_cast<const char32_t*>(src), src_len, output);
#endif
}

std::u16string WideToUTF16(WStringPiece wide) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return std::u16string(wide.begin(), wide.end());
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  return UTF32ToUTF16(
      StringPiece32(reinterpret_cast<const char32_t*>(wide.data()), 
                    wide.size()));
#endif
}

// Wide -> UTF32 ---------------------------------------------------------------
bool WideToUTF32(const wchar_t* src, size_t src_len, std::u32string* output) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return UTF16ToUTF32(reinterpret_cast<const char16_t*>(src), src_len, output);
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  output->assign(reinterpret_cast<const char32_t*>(src), src_len);
  return true;
#endif
}

std::u32string WideToUTF32(WStringPiece wide) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return UTF16ToUTF32(
      StringPiece16(reinterpret_cast<const char16_t*>(wide.data()), 
                    wide.size()));
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  return std::u32string(wide.begin(), wide.end());
#endif
}

// ASCII <-> UTF16 -------------------------------------------------------------
std::u16string ASCIIToUTF16(StringPiece ascii) {
  CR_DCHECK(IsStringASCII(ascii)) << ascii;
  return std::u16string(ascii.begin(), ascii.end());
}

std::string UTF16ToASCII(StringPiece16 utf16) {
  CR_DCHECK(IsStringASCII(utf16)) << UTF16ToUTF8(utf16);
  return std::string(utf16.begin(), utf16.end());
}

std::u32string ASCIIToUTF32(StringPiece ascii) {
  CR_DCHECK(IsStringASCII(ascii)) << ascii;
  return std::u32string(ascii.begin(), ascii.end());
}

std::string UTF32ToASCII(StringPiece32 utf32) {
  CR_DCHECK(IsStringASCII(utf32)) << UTF32ToUTF8(utf32);
  return std::string(utf32.begin(), utf32.end());
}

std::wstring ASCIIToWide(StringPiece ascii) {
  CR_DCHECK(IsStringASCII(ascii)) << ascii;
  return std::wstring(ascii.begin(), ascii.end());
}

std::string WideToASCII(WStringPiece wide) {
  CR_DCHECK(IsStringASCII(wide)) << wide;
  return std::string(wide.begin(), wide.end());
}

}  // namespace cr

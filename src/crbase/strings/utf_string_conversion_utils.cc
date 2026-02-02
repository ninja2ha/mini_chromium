// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/strings/utf_string_conversion_utils.h"

#include "crbase/third_party/icu/icu_utf.h"
#include "crbuild/build_config.h"

namespace cr {

// ReadUnicodeCharacter --------------------------------------------------------

bool ReadUnicodeCharacter(const char* src,
                          int32_t src_len,
                          int32_t* char_index,
                          uint32_t* code_point_out) {
  // U8_NEXT expects to be able to use -1 to signal an error, so we must
  // use a signed type for code_point.  But this function returns false
  // on error anyway, so code_point_out is unsigned.
  int32_t code_point;
  CBU8_NEXT(src, *char_index, src_len, code_point);
  *code_point_out = static_cast<uint32_t>(code_point);

  // The ICU macro above moves to the next char, we want to point to the last
  // char consumed.
  (*char_index)--;

  // Validate the decoded value.
  return IsValidCodepoint(code_point);
}

bool ReadUnicodeCharacter(const char16_t* src,
                          int32_t src_len,
                          int32_t* char_index,
                          uint32_t* code_point) {
  if (CBU16_IS_SURROGATE(src[*char_index])) {
    if (!CBU16_IS_SURROGATE_LEAD(src[*char_index]) ||
        *char_index + 1 >= src_len ||
        !CBU16_IS_TRAIL(src[*char_index + 1])) {
      // Invalid surrogate pair.
      return false;
    }

    // Valid surrogate pair.
    *code_point = CBU16_GET_SUPPLEMENTARY(src[*char_index],
                                          src[*char_index + 1]);
    (*char_index)++;
  } else {
    // Not a surrogate, just one 16-bit word.
    *code_point = src[*char_index];
  }

  return IsValidCodepoint(*code_point);
}

bool ReadUnicodeCharacter(const char32_t* src,
                          int32_t src_len,
                          int32_t* char_index,
                          uint32_t* code_point) {
  // Conversion is easy since the source is 32-bit.
  *code_point = src[*char_index];

  // Validate the value.
  return IsValidCodepoint(*code_point);
}

bool ReadUnicodeCharacter(const wchar_t* src,
                          int32_t src_len,
                          int32_t* char_index,
                          uint32_t* code_point) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return ReadUnicodeCharacter(
      reinterpret_cast<const char16_t*>(src), src_len, char_index, code_point);
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  // Conversion is easy since the source is 32-bit.
  *code_point = src[*char_index];

  // Validate the value.
  return IsValidCodepoint(*code_point);
#endif
}

// WriteUnicodeCharacter -------------------------------------------------------

size_t WriteUnicodeCharacter(uint32_t code_point, std::string* output) {
  if (code_point <= 0x7f) {
    // Fast path the common case of one byte.
    output->push_back(static_cast<char>(code_point));
    return 1;
  }


  // CBU8_APPEND_UNSAFE can append up to 4 bytes.
  size_t char_offset = output->length();
  size_t original_char_offset = char_offset;
  output->resize(char_offset + CBU8_MAX_LENGTH);

  CBU8_APPEND_UNSAFE(&(*output)[0], char_offset, code_point);

  // CBU8_APPEND_UNSAFE will advance our pointer past the inserted character, so
  // it will represent the new length of the string.
  output->resize(char_offset);
  return char_offset - original_char_offset;
}

size_t WriteUnicodeCharacter(uint32_t code_point, std::u16string* output) {
  if (CBU16_LENGTH(code_point) == 1) {
    // Thie code point is in the Basic Multilingual Plane (BMP).
    output->push_back(static_cast<char16_t>(code_point));
    return 1;
  }
  // Non-BMP characters use a double-character encoding.
  size_t char_offset = output->length();
  output->resize(char_offset + CBU16_MAX_LENGTH);
  CBU16_APPEND_UNSAFE(&(*output)[0], char_offset, code_point);
  return CBU16_MAX_LENGTH;
}

size_t WriteUnicodeCharacter(uint32_t code_point, std::u32string* output) {
  output->push_back(code_point);
  return 1;
}

size_t WriteUnicodeCharacter(uint32_t code_point, std::wstring* output) {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
  return WriteUnicodeCharacter(
      code_point, reinterpret_cast<std::u16string*>(output));
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
  output->push_back(code_point);
  return 1;
#endif
}

// Generalized Unicode converter -----------------------------------------------

template<typename CHAR>
void PrepareForUTF8Output(const CHAR* src,
                          size_t src_len,
                          std::string* output) {
  output->clear();
  if (src_len == 0)
    return;

  // Assume that the entire input is non-ASCII and will have 3 bytes per char.
  output->reserve(src_len * 3);
}

// Instantiate versions we know callers will need.
// wchar_t and char16_t are the same thing on Windows.
void PrepareForUTF8Output(const char16_t* src, 
                          size_t src_len, 
                          std::string* output) {
  PrepareForUTF8Output<>(src, src_len, output);
}

void PrepareForUTF8Output(const char32_t* src, 
                          size_t src_len, 
                          std::string* output) {
  PrepareForUTF8Output<>(src, src_len, output);
}

void PrepareForUTF8Output(const wchar_t* src, 
                          size_t src_len, 
                          std::string* output) {
  PrepareForUTF8Output<>(src, src_len, output);
}

template<typename STRING>
void PrepareForUTF16Or32Output(const char* src,
                               size_t src_len,
                               STRING* output) {
  output->clear();
  if (src_len == 0)
    return;

  // Otherwise assume that the UTF-8 sequences will have 2 bytes for each
  // character.
  output->reserve(src_len / 2);
}

// Instantiate versions we know callers will need.
// std::wstring and std::u16string are the same thing on Windows.
void PrepareForUTF16Or32Output(const char* src, 
                               size_t src_len, 
                               std::u16string* output) {
  PrepareForUTF16Or32Output<>(src, src_len, output);
}

void PrepareForUTF16Or32Output(const char* src, 
                               size_t src_len, 
                               std::u32string* output) {
  PrepareForUTF16Or32Output<>(src, src_len, output);
}

void PrepareForUTF16Or32Output(const char* src, 
                               size_t src_len, 
                               std::wstring* output) {
  PrepareForUTF16Or32Output<>(src, src_len, output);
}

}  // namespace cr

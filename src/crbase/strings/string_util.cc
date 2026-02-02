// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/strings/string_util.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#include <algorithm>
#include <limits>
#include <type_traits>
#include <vector>

#include "crbase/logging/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/stl_util.h"
#include "crbase/strings/string_util_internal.h"
#include "crbase/strings/utf_string_conversion_utils.h"
#include "crbase/third_party/icu/icu_utf.h"
#include "crbuild/build_config.h"

namespace cr {

bool IsWprintfFormatPortable(const wchar_t* format) {
  for (const wchar_t* position = format; *position != '\0'; ++position) {
    if (*position == '%') {
      bool in_specification = true;
      bool modifier_l = false;
      while (in_specification) {
        // Eat up characters until reaching a known specifier.
        if (*++position == '\0') {
          // The format string ended in the middle of a specification.  Call
          // it portable because no unportable specifications were found.  The
          // string is equally broken on all platforms.
          return true;
        }

        if (*position == 'l') {
          // 'l' is the only thing that can save the 's' and 'c' specifiers.
          modifier_l = true;
        } else if (((*position == 's' || *position == 'c') && !modifier_l) ||
                   *position == 'S' || *position == 'C' || *position == 'F' ||
                   *position == 'D' || *position == 'O' || *position == 'U') {
          // Not portable.
          return false;
        }

        if (wcschr(L"diouxXeEfgGaAcspn%", *position)) {
          // Portable, keep scanning the rest of the format string.
          in_specification = false;
        }
      }
    }
  }

  return true;
}

std::string ToLowerASCII(StringPiece str) {
  return internal::ToLowerASCIIImpl(str);
}

std::u16string ToLowerASCII(StringPiece16 str) {
  return internal::ToLowerASCIIImpl(str);
}

std::u32string ToLowerASCII(StringPiece32 str) {
  return internal::ToLowerASCIIImpl(str);
}

std::wstring ToLowerASCII(WStringPiece str) {
  return internal::ToLowerASCIIImpl(str);
}

std::string ToUpperASCII(StringPiece str) {
  return internal::ToUpperASCIIImpl(str);
}

std::u16string ToUpperASCII(StringPiece16 str) {
  return internal::ToUpperASCIIImpl(str);
}

std::u32string ToUpperASCII(StringPiece32 str) {
  return internal::ToUpperASCIIImpl(str);
}

std::wstring ToUpperASCII(WStringPiece str) {
  return internal::ToUpperASCIIImpl(str);
}

int CompareCaseInsensitiveASCII(StringPiece a, StringPiece b) {
  return internal::CompareCaseInsensitiveASCIIT(a, b);
}

int CompareCaseInsensitiveASCII(StringPiece16 a, StringPiece16 b) {
  return internal::CompareCaseInsensitiveASCIIT(a, b);
}

int CompareCaseInsensitiveASCII(StringPiece32 a, StringPiece32 b) {
  return internal::CompareCaseInsensitiveASCIIT(a, b);
}

int CompareCaseInsensitiveASCII(WStringPiece a, WStringPiece b) {
  return internal::CompareCaseInsensitiveASCIIT(a, b);
}

bool EqualsCaseInsensitiveASCII(StringPiece a, StringPiece b) {
  return a.size() == b.size() &&
         internal::CompareCaseInsensitiveASCIIT(a, b) == 0;
}

bool EqualsCaseInsensitiveASCII(StringPiece16 a, StringPiece16 b) {
  return a.size() == b.size() &&
         internal::CompareCaseInsensitiveASCIIT(a, b) == 0;
}

bool EqualsCaseInsensitiveASCII(StringPiece32 a, StringPiece32 b) {
  return a.size() == b.size() &&
         internal::CompareCaseInsensitiveASCIIT(a, b) == 0;
}

bool EqualsCaseInsensitiveASCII(WStringPiece a, WStringPiece b) {
  return a.size() == b.size() &&
    internal::CompareCaseInsensitiveASCIIT(a, b) == 0;
}

const std::string& EmptyString() {
  static const cr::NoDestructor<std::string> s;
  return *s;
}

const std::u16string& EmptyString16() {
  static const cr::NoDestructor<std::u16string> s16;
  return *s16;
}

const std::u32string& EmptyString32() {
  static const cr::NoDestructor<std::u32string> s32;
  return *s32;
}

const std::wstring& EmptyWString() {
  static const cr::NoDestructor<std::wstring> ws;
  return *ws;
}

const StringPiece& WhitespaceASCII() {
  static const cr::NoDestructor<StringPiece> sp([]{
    return kWhitespaceASCII;
  }());
  return *sp;
}

const StringPiece16& WhitespaceUTF16() {
  static const cr::NoDestructor<StringPiece16> sp([] {
    return kWhitespaceUTF16;
  }());
  return *sp;
}

const StringPiece32& WhitespaceUTF32() {
  static const cr::NoDestructor<StringPiece32> sp([] {
    return kWhitespaceUTF32;
  }());
  return *sp;
}

const WStringPiece& WhitespaceWide() {
  static const cr::NoDestructor<WStringPiece> sp([] {
#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
    return cr::AsWString(kWhitespaceUTF16);
#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)
    return cr::AsWString(kWhitespaceUTF32);
#endif
  }());
  return *sp;
}

bool ReplaceChars(StringPiece input,
                  StringPiece replace_chars,
                  StringPiece replace_with,
                  std::string* output) {
  return internal::ReplaceCharsT(input, replace_chars, replace_with, output);
}

bool ReplaceChars(StringPiece16 input,
                  StringPiece16 replace_chars,
                  StringPiece16 replace_with,
                  std::u16string* output) {
  return internal::ReplaceCharsT(input, replace_chars, replace_with, output);
}

bool ReplaceChars(StringPiece32 input,
                  StringPiece32 replace_chars,
                  StringPiece32 replace_with,
                  std::u32string* output) {
  return internal::ReplaceCharsT(input, replace_chars, replace_with, output);
}

bool ReplaceChars(WStringPiece input,
                  WStringPiece replace_chars,
                  WStringPiece replace_with,
                  std::wstring* output) {
  return internal::ReplaceCharsT(input, replace_chars, replace_with, output);
}

bool RemoveChars(StringPiece input,
                 StringPiece remove_chars,
                 std::string* output) {
  return internal::ReplaceCharsT(input, remove_chars, StringPiece(), output);
}

bool RemoveChars(StringPiece16 input,
                 StringPiece16 remove_chars,
                 std::u16string* output) {
  return internal::ReplaceCharsT(input, remove_chars, StringPiece16(), output);
}

bool RemoveChars(StringPiece32 input,
                 StringPiece32 remove_chars,
                 std::u32string* output) {
  return internal::ReplaceCharsT(input, remove_chars, StringPiece32(), output);
}

bool RemoveChars(WStringPiece input,
                 WStringPiece remove_chars,
                 std::wstring* output) {
  return internal::ReplaceCharsT(input, remove_chars, WStringPiece(), output);
}

bool TrimString(StringPiece input,
                StringPiece trim_chars,
                std::string* output) {
  return internal::TrimStringT(input, trim_chars, TRIM_ALL, output) !=
         TRIM_NONE;
}

bool TrimString(StringPiece16 input,
                StringPiece16 trim_chars,
                std::u16string* output) {
  return internal::TrimStringT(input, trim_chars, TRIM_ALL, output) !=
         TRIM_NONE;
}

bool TrimString(StringPiece32 input,
                StringPiece32 trim_chars,
                std::u32string* output) {
  return internal::TrimStringT(input, trim_chars, TRIM_ALL, output) !=
         TRIM_NONE;
}

bool TrimString(WStringPiece input,
                WStringPiece trim_chars,
                std::wstring* output) {
  return internal::TrimStringT(input, trim_chars, TRIM_ALL, output) !=
         TRIM_NONE;
}

StringPiece TrimString(StringPiece input,
                       StringPiece trim_chars,
                       TrimPositions positions) {
  return internal::TrimStringPieceT(input, trim_chars, positions);
}

StringPiece16 TrimString(StringPiece16 input,
                         StringPiece16 trim_chars,
                         TrimPositions positions) {
  return internal::TrimStringPieceT(input, trim_chars, positions);
}

StringPiece32 TrimString(StringPiece32 input,
                         StringPiece32 trim_chars,
                         TrimPositions positions) {
  return internal::TrimStringPieceT(input, trim_chars, positions);
}

WStringPiece TrimString(WStringPiece input,
                        WStringPiece trim_chars,
                        TrimPositions positions) {
  return internal::TrimStringPieceT(input, trim_chars, positions);
}

void TruncateUTF8ToByteSize(const std::string& input,
                            const size_t byte_size,
                            std::string* output) {
  CR_DCHECK(output);
  if (byte_size > input.length()) {
    *output = input;
    return;
  }
  CR_DCHECK(byte_size <=
            static_cast<uint32_t>(std::numeric_limits<int32_t>::max()));
  // Note: This cast is necessary because CBU8_NEXT uses int32_ts.
  int32_t truncation_length = static_cast<int32_t>(byte_size);
  int32_t char_index = truncation_length - 1;
  const char* data = input.data();

  // Using CBU8, we will move backwards from the truncation point
  // to the beginning of the string looking for a valid UTF8
  // character.  Once a full UTF8 character is found, we will
  // truncate the string to the end of that character.
  while (char_index >= 0) {
    int32_t prev = char_index;
    cr_icu::UChar32 code_point = 0;
    CBU8_NEXT(data, char_index, truncation_length, code_point);
    if (!IsValidCharacter(code_point) ||
        !IsValidCodepoint(code_point)) {
      char_index = prev - 1;
    } else {
      break;
    }
  }

  if (char_index >= 0 )
    *output = input.substr(0, char_index);
  else
    output->clear();
}

TrimPositions TrimWhitespaceASCII(StringPiece input,
                                  TrimPositions positions,
                                  std::string* output) {
  return internal::TrimStringT(input, WhitespaceASCII(), positions,
                               output);
}

StringPiece TrimWhitespaceASCII(StringPiece input, TrimPositions positions) {
  return internal::TrimStringPieceT(input, WhitespaceASCII(), positions);
}

TrimPositions TrimWhitespace(StringPiece16 input,
                             TrimPositions positions,
                             std::u16string* output) {
  return internal::TrimStringT(input, WhitespaceUTF16(),
                               positions, output);
}

StringPiece16 TrimWhitespace(StringPiece16 input,
                             TrimPositions positions) {
  return internal::TrimStringPieceT(input, WhitespaceUTF16(), positions);
}

TrimPositions TrimWhitespace(StringPiece32 input,
                             TrimPositions positions,
                             std::u32string* output) {
  return internal::TrimStringT(input, WhitespaceUTF32(), positions, output);
}

StringPiece32 TrimWhitespace(StringPiece32 input,
                             TrimPositions positions) {
  return internal::TrimStringPieceT(input, WhitespaceUTF32(), positions);
}

TrimPositions TrimWhitespace(WStringPiece input,
                             TrimPositions positions,
                             std::wstring* output) {
  return internal::TrimStringT(input, WhitespaceWide(), positions, output);
}

WStringPiece TrimWhitespace(WStringPiece input, TrimPositions positions) {
  return internal::TrimStringPieceT(input, WhitespaceWide(), positions);
}

std::string CollapseWhitespaceASCII(StringPiece text,
                                    bool trim_sequences_with_line_breaks) {
  return internal::CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

std::u16string CollapseWhitespace(StringPiece16 text,
                                  bool trim_sequences_with_line_breaks) {
  return internal::CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

std::u32string CollapseWhitespace(StringPiece32 text,
                                  bool trim_sequences_with_line_breaks) {
  return internal::CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

std::wstring CollapseWhitespaceASCII(WStringPiece text,
                                     bool trim_sequences_with_line_breaks) {
  return internal::CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

bool ContainsOnlyChars(StringPiece input, StringPiece characters) {
  return input.find_first_not_of(characters) == StringPiece::npos;
}

bool ContainsOnlyChars(StringPiece16 input, StringPiece16 characters) {
  return input.find_first_not_of(characters) == StringPiece16::npos;
}

bool ContainsOnlyChars(StringPiece32 input, StringPiece32 characters) {
  return input.find_first_not_of(characters) == StringPiece16::npos;
}

bool ContainsOnlyChars(WStringPiece input, WStringPiece characters) {
  return input.find_first_not_of(characters) == WStringPiece::npos;
}

bool IsStringASCII(StringPiece str) {
  return internal::DoIsStringASCII(str.data(), str.length());
}

bool IsStringASCII(StringPiece16 str) {
  return internal::DoIsStringASCII(str.data(), str.length());
}

bool IsStringASCII(StringPiece32 str) {
  return internal::DoIsStringASCII(str.data(), str.length());
}

bool IsStringASCII(WStringPiece str) {
  return internal::DoIsStringASCII(str.data(), str.length());
}

bool IsStringUTF8(StringPiece str) {
  return internal::DoIsStringUTF8<IsValidCharacter>(str);
}

bool IsStringUTF8AllowingNoncharacters(StringPiece str) {
  return internal::DoIsStringUTF8<IsValidCodepoint>(str);
}

bool LowerCaseEqualsASCII(StringPiece str, StringPiece lowercase_ascii) {
  return internal::DoLowerCaseEqualsASCII(str, lowercase_ascii);
}

bool LowerCaseEqualsASCII(StringPiece16 str, StringPiece lowercase_ascii) {
  return internal::DoLowerCaseEqualsASCII(str, lowercase_ascii);
}

bool LowerCaseEqualsASCII(StringPiece32 str, StringPiece lowercase_ascii) {
  return internal::DoLowerCaseEqualsASCII(str, lowercase_ascii);
}

bool LowerCaseEqualsASCII(WStringPiece str, StringPiece lowercase_ascii) {
  return internal::DoLowerCaseEqualsASCII(str, lowercase_ascii);
}

bool EqualsASCII(StringPiece16 str, StringPiece ascii) {
  return std::equal(ascii.begin(), ascii.end(), str.begin(), str.end());
}

bool EqualsASCII(StringPiece32 str, StringPiece ascii) {
  return std::equal(ascii.begin(), ascii.end(), str.begin(), str.end());
}

bool EqualsASCII(WStringPiece str, StringPiece ascii) {
  return std::equal(ascii.begin(), ascii.end(), str.begin(), str.end());
}

bool StartsWith(StringPiece str,
                StringPiece search_for,
                CompareCase case_sensitivity) {
  return internal::StartsWithT(str, search_for, case_sensitivity);
}

bool StartsWith(StringPiece16 str,
                StringPiece16 search_for,
                CompareCase case_sensitivity) {
  return internal::StartsWithT(str, search_for, case_sensitivity);
}

bool StartsWith(StringPiece32 str,
                StringPiece32 search_for,
                CompareCase case_sensitivity) {
  return internal::StartsWithT(str, search_for, case_sensitivity);
}

bool StartsWith(WStringPiece str,
                WStringPiece search_for,
                CompareCase case_sensitivity) {
  return internal::StartsWithT(str, search_for, case_sensitivity);
}

bool EndsWith(StringPiece str,
              StringPiece search_for,
              CompareCase case_sensitivity) {
  return internal::EndsWithT(str, search_for, case_sensitivity);
}

bool EndsWith(StringPiece16 str,
              StringPiece16 search_for,
              CompareCase case_sensitivity) {
  return internal::EndsWithT(str, search_for, case_sensitivity);
}

bool EndsWith(StringPiece32 str,
              StringPiece32 search_for,
              CompareCase case_sensitivity) {
  return internal::EndsWithT(str, search_for, case_sensitivity);
}

bool EndsWith(WStringPiece str,
              WStringPiece search_for,
              CompareCase case_sensitivity) {
  return internal::EndsWithT(str, search_for, case_sensitivity);
}

char HexDigitToInt(wchar_t c) {
  CR_DCHECK(IsHexDigit(c));
  if (c >= '0' && c <= '9')
    return static_cast<char>(c - '0');
  if (c >= 'A' && c <= 'F')
    return static_cast<char>(c - 'A' + 10);
  if (c >= 'a' && c <= 'f')
    return static_cast<char>(c - 'a' + 10);
  return 0;
}

bool IsUnicodeWhitespace(int32_t c) {
  // kWhitespaceWide is a NULL-terminated string
  for (const char32_t* cur = kWhitespaceUTF32; *cur; ++cur) {
    if (*cur == c)
      return true;
  }
  return false;
}

static const char* const kByteStringsUnlocalized[] = {
  " B",
  " kB",
  " MB",
  " GB",
  " TB",
  " PB"
};

std::string FormatBytesUnlocalized(int64_t bytes) {
  double unit_amount = static_cast<double>(bytes);
  size_t dimension = 0;
  const int kKilo = 1024;
  while (unit_amount >= kKilo &&
         dimension < cr::size(kByteStringsUnlocalized) - 1) {
    unit_amount /= kKilo;
    dimension++;
  }

  char buf[64];
  if (bytes != 0 && dimension > 0 && unit_amount < 100) {
    cr::snprintf(buf, cr::size(buf), "%.1lf%s", unit_amount,
                 kByteStringsUnlocalized[dimension]);
  } else {
    cr::snprintf(buf, cr::size(buf), "%.0lf%s", unit_amount,
                 kByteStringsUnlocalized[dimension]);
  }

  return std::string(buf);
}


void ReplaceFirstSubstringAfterOffset(std::string* str,
                                      size_t start_offset,
                                      StringPiece find_this,
                                      StringPiece replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_FIRST);
}

void ReplaceFirstSubstringAfterOffset(std::u16string* str,
                                      size_t start_offset,
                                      StringPiece16 find_this,
                                      StringPiece16 replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_FIRST);
}

void ReplaceFirstSubstringAfterOffset(std::u32string* str,
                                      size_t start_offset,
                                      StringPiece32 find_this,
                                      StringPiece32 replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_FIRST);
}

void ReplaceFirstSubstringAfterOffset(std::wstring* str,
                                      size_t start_offset,
                                      WStringPiece find_this,
                                      WStringPiece replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_FIRST);
}

void ReplaceSubstringsAfterOffset(std::string* str,
                                  size_t start_offset,
                                  StringPiece find_this,
                                  StringPiece replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_ALL);
}

void ReplaceSubstringsAfterOffset(std::u16string* str,
                                  size_t start_offset,
                                  StringPiece16 find_this,
                                  StringPiece16 replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_ALL);
}

void ReplaceSubstringsAfterOffset(std::u32string* str,
                                  size_t start_offset,
                                  StringPiece32 find_this,
                                  StringPiece32 replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_ALL);
}

void ReplaceSubstringsAfterOffset(std::wstring* str,
                                  size_t start_offset,
                                  WStringPiece find_this,
                                  WStringPiece replace_with) {
  internal::DoReplaceMatchesAfterOffset(
      str, start_offset, internal::MakeSubstringMatcher(find_this),
      replace_with, internal::ReplaceType::REPLACE_ALL);
}

char* WriteInto(std::string* str, size_t length_with_null) {
  return internal::WriteIntoT(str, length_with_null);
}

char16_t* WriteInto(std::u16string* str, size_t length_with_null) {
  return internal::WriteIntoT(str, length_with_null);
}

char32_t* WriteInto(std::u32string* str, size_t length_with_null) {
  return internal::WriteIntoT(str, length_with_null);
}

wchar_t* WriteInto(std::wstring* str, size_t length_with_null) {
  return internal::WriteIntoT(str, length_with_null);
}

std::string JoinString(Span<const std::string> parts, StringPiece separator) {
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

std::string JoinString(Span<const StringPiece> parts, StringPiece separator) {
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

std::string JoinString(std::initializer_list<StringPiece> parts,
                       StringPiece separator) {
  return internal::JoinStringT(parts, separator);
}

std::u16string JoinString(std::initializer_list<StringPiece16> parts,
                          StringPiece16 separator) {
  return internal::JoinStringT(parts, separator);
}

std::u32string JoinString(std::initializer_list<StringPiece32> parts,
                          StringPiece32 separator) {
  return internal::JoinStringT(parts, separator);
}

std::wstring JoinString(std::initializer_list<WStringPiece> parts,
                        WStringPiece separator) {
  return internal::JoinStringT(parts, separator);
}

std::string ReplaceStringPlaceholders(StringPiece format_string,
                                      const std::vector<std::string>& subst,
                                      std::vector<size_t>* offsets) {
  return internal::DoReplaceStringPlaceholders(format_string, subst, offsets);
}

std::u16string ReplaceStringPlaceholders(
    StringPiece16 format_string,
    const std::vector<std::u16string>& subst,
    std::vector<size_t>* offsets) {
  return internal::DoReplaceStringPlaceholders(format_string, subst, offsets);
}

std::u32string ReplaceStringPlaceholders(
    StringPiece32 format_string,
    const std::vector<std::u32string>& subst,
    std::vector<size_t>* offsets) {
  return internal::DoReplaceStringPlaceholders(format_string, subst, offsets);
}

std::wstring ReplaceStringPlaceholders(WStringPiece format_string,
                                       const std::vector<std::wstring>& subst,
                                       std::vector<size_t>* offsets) {
  return internal::DoReplaceStringPlaceholders(format_string, subst, offsets);
}

std::u16string ReplaceStringPlaceholders(const std::u16string& format_string,
                                         const std::u16string& a,
                                         size_t* offset) {
  std::vector<size_t> offsets;
  std::u16string result =
      ReplaceStringPlaceholders(format_string, {a}, &offsets);

  CR_DCHECK(1U == offsets.size());
  if (offset)
    *offset = offsets[0];
  return result;
}

std::u32string ReplaceStringPlaceholders(const std::u32string& format_string,
                                         const std::u32string& a,
                                         size_t* offset) {
  std::vector<size_t> offsets;
  std::u32string result =
      ReplaceStringPlaceholders(format_string, {a}, &offsets);

  CR_DCHECK(1U == offsets.size());
  if (offset)
    *offset = offsets[0];
  return result;
}

size_t strlcpy(char* dst, const char* src, size_t dst_size) {
  return internal::lcpyT(dst, src, dst_size);
}
size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size) {
  return internal::lcpyT(dst, src, dst_size);
}

// doc: https://www.ibm.com/docs/zh-tw/i/7.5.0?topic=tables-unicode-uppercase-lowercase-conversion-mapping-table
uint32_t ToLower(uint32_t c) {
  if (c >= 0x0041 && c <= 0x005A) return c + 0x20;
  if (c >= 0x00C0 && c <= 0x00DE) return c + 0x20;

  int32_t hiword = c >> 8;
  if (hiword > 15) goto label_16;
  if (hiword > 3) goto label_4;
  if (hiword == 3) goto label_3;
  if (c >= 0x0100 && c <= 0x0216) return (c & 1) == 1 ? c : c + 1;
  return c;
label_3:
  if (c == 0x0386) return 0x03AC;
  if (c == 0x0388) return 0x03AD;
  if (c == 0x0389) return 0x03AE;
  if (c == 0x038A) return 0x03AF;
  if (c == 0x038C) return 0x03CC;
  if (c == 0x038E) return 0x03CD;
  if (c == 0x038F) return 0x03CE;
  if (c >= 0x0391 && c <= 0x03AB) return (c == 0x03A2) ? c : c + 0x20;
  if (c >= 0x03E2 && c <= 0x03EE) return (c & 1) == 1 ? c : c + 1;
  return c;
label_4:
  if (c >= 0x0401 && c <= 0x040F) return c + 0x50;
  if (c >= 0x0410 && c <= 0x042F) return c + 0x20;
  if (c >= 0x0460 && c <= 0x0480) return (c & 1) == 1 ? c : c + 1;
  if (c >= 0x0490 && c <= 0x04BE) return (c & 1) == 1 ? c : c + 1;
  if (c >= 0x04C1 && c <= 0x04F8) return (c == 0x04C5 || c == 0x04C6) ? c :
                                             (c & 1) == 1 ? c : c + 1;
  if (c >= 0x0531 && c <= 0x0556) return c + 0x30;
  return c;
label_16:
  if (c >= 0x10A0 && c <= 0x10C5) return c + 0x30;
  if (c >= 0x1E00 && c <= 0x1E94) return (c & 1) == 1 ? c : c + 1;
  if (c >= 0x1EA0 && c <= 0x1EF8) return (c & 1) == 1 ? c : c + 1;
  if (c >= 0x1F08 && c <= 0x1F0F) return c - 8;
  if (c >= 0x1F18 && c <= 0x1F1D) return c - 8;
  if (c >= 0x1F28 && c <= 0x1F2F) return c - 8;
  if (c >= 0x1F48 && c <= 0x1F4D) return c - 8;
  if (c >= 0x1F59 && c <= 0x1F5F) return c - 8;
  if (c >= 0x1F68 && c <= 0x1F6F) return c - 8;
  if (c >= 0x1F88 && c <= 0x1F8F) return c - 8;
  if (c >= 0x1F98 && c <= 0x1F9F) return c - 8;
  if (c >= 0x1FA8 && c <= 0x1FAF) return c - 8;
  if (c == 0x1FB8 || c == 0x1FB9) return c - 8;
  if (c == 0x1FD8 || c == 0x1FD9) return c - 8;
  if (c == 0x1FE8 || c == 0x1FE9) return c - 8;

  if (c >= 0x24B6 && c <= 0x24CF) return c + 0x1A;
  if (c >= 0xFF21 && c <= 0xFF3A) return c + 0x20;
  return c;
}

}  // namespace cr

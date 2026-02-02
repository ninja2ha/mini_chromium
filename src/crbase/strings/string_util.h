// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169
//
// This file defines utility functions for working with strings.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_H_

#include <ctype.h>
#include <stdarg.h>   // va_list
#include <stddef.h>
#include <stdint.h>

#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

#include "crbase/base_export.h"
#include "crbase/logging/logging.h"
#include "crbase/containers/span.h"
#include "crbase/stl_util.h"
#include "crbase/strings/string_piece.h"  // For implicit conversions.
#include "crbuild/compiler_specific.h"
#include "crbuild/build_config.h"

namespace cr {

// C standard-library functions that aren't cross-platform are provided as
// "cr::...", and their prototypes are listed below. These functions are
// then implemented as inline calls to the platform-specific equivalents in the
// platform-specific headers.

// Wrapper for vsnprintf that always null-terminates and always returns the
// number of characters that would be in an untruncated formatted
// string, even when truncation occurs.
int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments)
    CR_PRINTF_FORMAT(3, 0);

// Some of these implementations need to be inlined.

// We separate the declaration from the implementation of this inline
// function just so the CR_PRINTF_FORMAT works.
inline int snprintf(char* buffer, size_t size, const char* format, ...)
    CR_PRINTF_FORMAT(3, 4);
inline int snprintf(char* buffer, size_t size, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsnprintf(buffer, size, format, arguments);
  va_end(arguments);
  return result;
}

// BSD-style safe and consistent string copy functions.
// Copies |src| to |dst|, where |dst_size| is the total allocated size of |dst|.
// Copies at most |dst_size|-1 characters, and always NULL terminates |dst|, as
// long as |dst_size| is not 0.  Returns the length of |src| in characters.
// If the return value is >= dst_size, then the output was truncated.
// NOTE: All sizes are in number of characters, NOT in bytes.
CRBASE_EXPORT size_t strlcpy(char* dst, const char* src, size_t dst_size);
CRBASE_EXPORT size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size);

// Scan a wprintf format string to determine whether it's portable across a
// variety of systems.  This function only checks that the conversion
// specifiers used by the format string are supported and have the same meaning
// on a variety of systems.  It doesn't check for other errors that might occur
// within a format string.
//
// Nonportable conversion specifiers for wprintf are:
//  - 's' and 'c' without an 'l' length modifier.  %s and %c operate on char
//     data on all systems except Windows, which treat them as wchar_t data.
//     Use %ls and %lc for wchar_t data instead.
//  - 'S' and 'C', which operate on wchar_t data on all systems except Windows,
//     which treat them as char data.  Use %ls and %lc for wchar_t data
//     instead.
//  - 'F', which is not identified by Windows wprintf documentation.
//  - 'D', 'O', and 'U', which are deprecated and not available on all systems.
//     Use %ld, %lo, and %lu instead.
//
// Note that there is no portable conversion specifier for char data when
// working with wprintf.
//
// This function is intended to be called from cr::vswprintf.
CRBASE_EXPORT bool IsWprintfFormatPortable(const wchar_t* format);

// Simplified implementation of C++20's std::basic_string_view(It, End).
// Reference: https://wg21.link/string.view.cons
template <typename CharT, typename Iter>
constexpr BasicStringPiece<CharT> MakeBasicStringPiece(Iter begin, Iter end) {
  CR_DCHECK((end - begin) >= 0);
  return BasicStringPiece<CharT>(
      cr::to_address(begin), static_cast<size_t>(end - begin));
}

// Explicit instantiations of MakeBasicStringPiece for the BasicStringPiece
// aliases defined in base/strings/string_piece_forward.h
template <typename Iter>
constexpr StringPiece MakeStringPiece(Iter begin, Iter end) {
  return MakeBasicStringPiece<char>(begin, end);
}

template <typename Iter>
constexpr StringPiece16 MakeStringPiece16(Iter begin, Iter end) {
  return MakeBasicStringPiece<char16_t>(begin, end);
}

template <typename Iter>
constexpr WStringPiece MakeWStringPiece(Iter begin, Iter end) {
  return MakeBasicStringPiece<wchar_t>(begin, end);
}

// ASCII-specific tolower.  The standard library's tolower is locale sensitive,
// so we don't want to use it here.
template <typename CharT,
          typename = std::enable_if_t<std::is_integral<CharT>::value>>
CharT ToLowerASCII(CharT c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// ASCII-specific toupper.  The standard library's toupper is locale sensitive,
// so we don't want to use it here.
template <typename CharT,
          typename = std::enable_if_t<std::is_integral<CharT>::value>>
CharT ToUpperASCII(CharT c) {
  return (c >= 'a' && c <= 'z') ? (c + ('A' - 'a')) : c;
}

// UNICODE-specific tolower
CRBASE_EXPORT int32_t ToLower(int32_t c);

// Converts the given string to it's ASCII-lowercase equivalent.
CRBASE_EXPORT std::string ToLowerASCII(StringPiece str);
CRBASE_EXPORT std::u16string ToLowerASCII(StringPiece16 str);
CRBASE_EXPORT std::u32string ToLowerASCII(StringPiece32 str);
CRBASE_EXPORT std::wstring ToLowerASCII(WStringPiece str);

// Converts the given string to it's ASCII-uppercase equivalent.
CRBASE_EXPORT std::string ToUpperASCII(StringPiece str);
CRBASE_EXPORT std::u16string ToUpperASCII(StringPiece16 str);
CRBASE_EXPORT std::u32string ToUpperASCII(StringPiece32 str);
CRBASE_EXPORT std::wstring ToUpperASCII(WStringPiece str);

// Functor for case-insensitive ASCII comparisons for STL algorithms like
// std::search.
//
// Note that a full Unicode version of this functor is not possible to write
// because case mappings might change the number of characters, depend on
// context (combining accents), and require handling UTF-16. If you need
// proper Unicode support, use cr::i18n::ToLower/FoldCase and then just
// use a normal operator== on the result.
template<typename Char> struct CaseInsensitiveCompareASCII {
 public:
  bool operator()(Char x, Char y) const {
    return ToLowerASCII(x) == ToLowerASCII(y);
  }
};

// Like strcasecmp for case-insensitive ASCII characters only. Returns:
//   -1  (a < b)
//    0  (a == b)
//    1  (a > b)
// (unlike strcasecmp which can return values greater or less than 1/-1). For
// full Unicode support, use cr::i18n::ToLower or cr::i18h::FoldCase
// and then just call the normal string operators on the result.
CRBASE_EXPORT int CompareCaseInsensitiveASCII(StringPiece a, StringPiece b);
CRBASE_EXPORT int CompareCaseInsensitiveASCII(StringPiece16 a, StringPiece16 b);
CRBASE_EXPORT int CompareCaseInsensitiveASCII(StringPiece32 a, StringPiece32 b);
CRBASE_EXPORT int CompareCaseInsensitiveASCII(WStringPiece a, WStringPiece b);

// Equality for ASCII case-insensitive comparisons. For full Unicode support,
// use cr::i18n::ToLower or cr::i18h::FoldCase and then compare with either
// == or !=.
CRBASE_EXPORT bool EqualsCaseInsensitiveASCII(StringPiece a, StringPiece b);
CRBASE_EXPORT bool EqualsCaseInsensitiveASCII(StringPiece16 a, StringPiece16 b);
CRBASE_EXPORT bool EqualsCaseInsensitiveASCII(StringPiece32 a, StringPiece32 b);
CRBASE_EXPORT bool EqualsCaseInsensitiveASCII(WStringPiece a, WStringPiece b);

// These threadsafe functions return references to globally unique empty
// strings.
//
// It is likely faster to construct a new empty string object (just a few
// instructions to set the length to 0) than to get the empty string instance
// returned by these functions (which requires threadsafe static access).
//
// Therefore, DO NOT USE THESE AS A GENERAL-PURPOSE SUBSTITUTE FOR DEFAULT
// CONSTRUCTORS. There is only one case where you should use these: functions
// which need to return a string by reference (e.g. as a class member
// accessor), and don't have an empty string to use (e.g. in an error case).
// These should not be used as initializers, function arguments, or return
// values for functions which return by value or outparam.
CRBASE_EXPORT const std::string& EmptyString();
CRBASE_EXPORT const std::u16string& EmptyString16();
CRBASE_EXPORT const std::u32string& EmptyString32();
CRBASE_EXPORT const std::wstring& EmptyWString();

// Contains the set of characters representing whitespace in the corresponding
// encoding. Null-terminated. The ASCII versions are the whitespaces as defined
// by HTML5, and don't include control characters.
CRBASE_EXPORT extern const char kWhitespaceASCII[];
CRBASE_EXPORT extern const char32_t kWhitespaceUTF32[];  // Includes Unicode.
CRBASE_EXPORT extern const char16_t kWhitespaceUTF16[];  // Includes Unicode.
CRBASE_EXPORT extern const char16_t
    kWhitespaceNoCrLfUTF16[];  // Unicode w/o CR/LF.
CRBASE_EXPORT extern const char16_t kWhitespaceASCIIAs16[];  // No unicode.

CRBASE_EXPORT const StringPiece& WhitespaceASCII();
CRBASE_EXPORT const StringPiece16& WhitespaceUTF16();
CRBASE_EXPORT const StringPiece32& WhitespaceUTF32();
CRBASE_EXPORT const WStringPiece& WhitespaceWide();

// Null-terminated string representing the UTF-8 byte order mark.
CRBASE_EXPORT extern const char kUtf8ByteOrderMark[];

// Removes characters in |remove_chars| from anywhere in |input|.  Returns true
// if any characters were removed.  |remove_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
CRBASE_EXPORT bool RemoveChars(StringPiece input,
                               StringPiece remove_chars,
                               std::string* output);
CRBASE_EXPORT bool RemoveChars(StringPiece16 input,
                               StringPiece16 remove_chars,
                               std::u16string* output);
CRBASE_EXPORT bool RemoveChars(StringPiece32 input,
                               StringPiece32 remove_chars,
                               std::u32string* output);
CRBASE_EXPORT bool RemoveChars(WStringPiece input,
                               WStringPiece remove_chars,
                               std::wstring* output);

// Replaces characters in |replace_chars| from anywhere in |input| with
// |replace_with|.  Each character in |replace_chars| will be replaced with
// the |replace_with| string.  Returns true if any characters were replaced.
// |replace_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
CRBASE_EXPORT bool ReplaceChars(StringPiece input,
                                StringPiece replace_chars,
                                StringPiece replace_with,
                                std::string* output);
CRBASE_EXPORT bool ReplaceChars(StringPiece16 input,
                                StringPiece16 replace_chars,
                                StringPiece16 replace_with,
                                std::u16string* output);
CRBASE_EXPORT bool ReplaceChars(StringPiece32 input,
                                StringPiece32 replace_chars,
                                StringPiece32 replace_with,
                                std::u32string* output);
CRBASE_EXPORT bool ReplaceChars(WStringPiece input,
                                WStringPiece replace_chars,
                                WStringPiece replace_with,
                                std::wstring* output);

enum TrimPositions {
  TRIM_NONE     = 0,
  TRIM_LEADING  = 1 << 0,
  TRIM_TRAILING = 1 << 1,
  TRIM_ALL      = TRIM_LEADING | TRIM_TRAILING,
};

// Removes characters in |trim_chars| from the beginning and end of |input|.
// The 8-bit version only works on 8-bit characters, not UTF-8. Returns true if
// any characters were removed.
//
// It is safe to use the same variable for both |input| and |output| (this is
// the normal usage to trim in-place).
CRBASE_EXPORT bool TrimString(StringPiece input,
                              StringPiece trim_chars,
                              std::string* output);
CRBASE_EXPORT bool TrimString(StringPiece16 input,
                              StringPiece16 trim_chars,
                              std::u16string* output);
CRBASE_EXPORT bool TrimString(StringPiece32 input,
                              StringPiece32 trim_chars,
                              std::u32string* output);
CRBASE_EXPORT bool TrimString(WStringPiece input,
                              WStringPiece trim_chars,
                              std::wstring* output);

// StringPiece versions of the above. The returned pieces refer to the original
// buffer.
CRBASE_EXPORT StringPiece TrimString(StringPiece input,
                                     StringPiece trim_chars,
                                     TrimPositions positions);
CRBASE_EXPORT StringPiece16 TrimString(StringPiece16 input,
                                       StringPiece16 trim_chars,
                                       TrimPositions positions);
CRBASE_EXPORT StringPiece32 TrimString(StringPiece32 input,
                                       StringPiece32 trim_chars,
                                       TrimPositions positions);
CRBASE_EXPORT WStringPiece TrimString(WStringPiece input,
                                      WStringPiece trim_chars,
                                      TrimPositions positions);

// Truncates a string to the nearest UTF-8 character that will leave
// the string less than or equal to the specified byte size.
CRBASE_EXPORT void TruncateUTF8ToByteSize(const std::string& input,
                                          const size_t byte_size,
                                          std::string* output);

// Trims any whitespace from either end of the input string.
//
// The StringPiece versions return a substring referencing the input buffer.
// The ASCII versions look only for ASCII whitespace.
//
// The std::string versions return where whitespace was found.
// NOTE: Safe to use the same variable for both input and output.
CRBASE_EXPORT TrimPositions TrimWhitespaceASCII(StringPiece input,
                                                TrimPositions positions,
                                                std::string* output);
CRBASE_EXPORT StringPiece TrimWhitespaceASCII(StringPiece input,
                                              TrimPositions positions);

CRBASE_EXPORT TrimPositions TrimWhitespace(StringPiece16 input,
                                           TrimPositions positions,
                                           std::u16string* output);
CRBASE_EXPORT StringPiece16 TrimWhitespace(StringPiece16 input,
                                           TrimPositions positions);

CRBASE_EXPORT TrimPositions TrimWhitespace(StringPiece32 input,
                                           TrimPositions positions,
                                           std::u32string* output);
CRBASE_EXPORT StringPiece32 TrimWhitespace(StringPiece32 input,
                                           TrimPositions positions);

CRBASE_EXPORT TrimPositions TrimWhitespace(WStringPiece input,
                                           TrimPositions positions,
                                           std::wstring* output);
CRBASE_EXPORT WStringPiece TrimWhitespace(WStringPiece input,
                                          TrimPositions positions);

// Searches for CR or LF characters.  Removes all contiguous whitespace
// strings that contain them.  This is useful when trying to deal with text
// copied from terminals.
// Returns |text|, with the following three transformations:
// (1) Leading and trailing whitespace is trimmed.
// (2) If |trim_sequences_with_line_breaks| is true, any other whitespace
//     sequences containing a CR or LF are trimmed.
// (3) All other whitespace sequences are converted to single spaces.
CRBASE_EXPORT std::string CollapseWhitespaceASCII(
    StringPiece text,
    bool trim_sequences_with_line_breaks);
CRBASE_EXPORT std::u16string CollapseWhitespace(
    StringPiece16 text,
    bool trim_sequences_with_line_breaks);
CRBASE_EXPORT std::u32string CollapseWhitespace(
    StringPiece32 text,
    bool trim_sequences_with_line_breaks);
CRBASE_EXPORT std::wstring CollapseWhitespaceASCII(
    WStringPiece text,
    bool trim_sequences_with_line_breaks);

// Returns true if |input| is empty or contains only characters found in
// |characters|.
CRBASE_EXPORT bool ContainsOnlyChars(StringPiece input, StringPiece characters);
CRBASE_EXPORT bool ContainsOnlyChars(StringPiece16 input,
                                     StringPiece16 characters);
CRBASE_EXPORT bool ContainsOnlyChars(StringPiece32 input,
                                     StringPiece32 characters);
CRBASE_EXPORT bool ContainsOnlyChars(WStringPiece input,
                                     WStringPiece characters);

// Returns true if |str| is structurally valid UTF-8 and also doesn't
// contain any non-character code point (e.g. U+10FFFE). Prohibiting
// non-characters increases the likelihood of detecting non-UTF-8 in
// real-world text, for callers which do not need to accept
// non-characters in strings.
CRBASE_EXPORT bool IsStringUTF8(StringPiece str);

// Returns true if |str| contains valid UTF-8, allowing non-character
// code points.
CRBASE_EXPORT bool IsStringUTF8AllowingNoncharacters(StringPiece str);

// Returns true if |str| contains only valid ASCII character values.
// Note 1: IsStringASCII executes in time determined solely by the
// length of the string, not by its contents, so it is robust against
// timing attacks for all strings of equal length.
// Note 2: IsStringASCII assumes the input is likely all ASCII, and
// does not leave early if it is not the case.
CRBASE_EXPORT bool IsStringASCII(StringPiece str);
CRBASE_EXPORT bool IsStringASCII(StringPiece16 str);
CRBASE_EXPORT bool IsStringASCII(StringPiece32 str);
CRBASE_EXPORT bool IsStringASCII(WStringPiece str);

// Compare the lower-case form of the given string against the given
// previously-lower-cased ASCII string (typically a constant).
CRBASE_EXPORT bool LowerCaseEqualsASCII(StringPiece str,
                                        StringPiece lowercase_ascii);
CRBASE_EXPORT bool LowerCaseEqualsASCII(StringPiece16 str,
                                        StringPiece lowercase_ascii);
CRBASE_EXPORT bool LowerCaseEqualsASCII(StringPiece32 str,
                                        StringPiece lowercase_ascii);
CRBASE_EXPORT bool LowerCaseEqualsASCII(WStringPiece str,
                                        StringPiece lowercase_ascii);

// Performs a case-sensitive string compare of the given 16-bit string against
// the given 8-bit ASCII string (typically a constant). The behavior is
// undefined if the |ascii| string is not ASCII.
CRBASE_EXPORT bool EqualsASCII(StringPiece16 str, StringPiece ascii);
CRBASE_EXPORT bool EqualsASCII(StringPiece32 str, StringPiece ascii);
CRBASE_EXPORT bool EqualsASCII(WStringPiece str, StringPiece ascii);

// Indicates case sensitivity of comparisons. Only ASCII case insensitivity
// is supported. Full Unicode case-insensitive conversions would need to go in
// base/i18n so it can use ICU.
//
// If you need to do Unicode-aware case-insensitive StartsWith/EndsWith, it's
// best to call cr::i18n::ToLower() or cr::i18n::FoldCase() (see
// base/i18n/case_conversion.h for usage advice) on the arguments, and then use
// the results to a case-sensitive comparison.
enum class CompareCase {
  SENSITIVE,
  INSENSITIVE_ASCII,
};

CRBASE_EXPORT bool StartsWith(
    StringPiece str,
    StringPiece search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);
CRBASE_EXPORT bool StartsWith(
    StringPiece16 str,
    StringPiece16 search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);
CRBASE_EXPORT bool StartsWith(
    StringPiece32 str,
    StringPiece32 search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);
CRBASE_EXPORT bool StartsWith(
    WStringPiece str,
    WStringPiece search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);
CRBASE_EXPORT bool EndsWith(
    StringPiece str,
    StringPiece search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);
CRBASE_EXPORT bool EndsWith(
    StringPiece16 str,
    StringPiece16 search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);
CRBASE_EXPORT bool EndsWith(
    StringPiece32 str,
    StringPiece32 search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);
CRBASE_EXPORT bool EndsWith(
    WStringPiece str,
    WStringPiece search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);

// Determines the type of ASCII character, independent of locale (the C
// library versions will change based on locale).
template <typename Char>
inline bool IsAsciiWhitespace(Char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\f';
}
template <typename Char>
inline bool IsAsciiAlpha(Char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
template <typename Char>
inline bool IsAsciiUpper(Char c) {
  return c >= 'A' && c <= 'Z';
}
template <typename Char>
inline bool IsAsciiLower(Char c) {
  return c >= 'a' && c <= 'z';
}
template <typename Char>
inline bool IsAsciiDigit(Char c) {
  return c >= '0' && c <= '9';
}
template <typename Char>
inline bool IsAsciiPrintable(Char c) {
  return c >= ' ' && c <= '~';
}

template <typename Char>
inline bool IsHexDigit(Char c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'F') ||
         (c >= 'a' && c <= 'f');
}

// Returns the integer corresponding to the given hex character. For example:
//    '4' -> 4
//    'a' -> 10
//    'B' -> 11
// Assumes the input is a valid hex character. DCHECKs in debug builds if not.
CRBASE_EXPORT char HexDigitToInt(wchar_t c);

// Returns true if it's a Unicode whitespace character.
CRBASE_EXPORT bool IsUnicodeWhitespace(int32_t c);

// Return a byte string in human-readable format with a unit suffix. Not
// appropriate for use in any UI; use of FormatBytes and friends in ui/base is
// highly recommended instead. TODO(avi): Figure out how to get callers to use
// FormatBytes instead; remove this.
CRBASE_EXPORT std::string FormatBytesUnlocalized(int64_t bytes);

// Starting at |start_offset| (usually 0), replace the first instance of
// |find_this| with |replace_with|.
CRBASE_EXPORT void ReplaceFirstSubstringAfterOffset(
    std::string* str,
    size_t start_offset,
    StringPiece find_this,
    StringPiece replace_with);
CRBASE_EXPORT void ReplaceFirstSubstringAfterOffset(
    std::u16string* str,
    size_t start_offset,
    StringPiece16 find_this,
    StringPiece16 replace_with);
CRBASE_EXPORT void ReplaceFirstSubstringAfterOffset(
    std::u32string* str,
    size_t start_offset,
    StringPiece32 find_this,
    StringPiece32 replace_with);
CRBASE_EXPORT void ReplaceFirstSubstringAfterOffset(
    std::wstring* str,
    size_t start_offset,
    WStringPiece find_this,
    WStringPiece replace_with);

// Starting at |start_offset| (usually 0), look through |str| and replace all
// instances of |find_this| with |replace_with|.
//
// This does entire substrings; use std::replace in <algorithm> for single
// characters, for example:
//   std::replace(str.begin(), str.end(), 'a', 'b');
CRBASE_EXPORT void ReplaceSubstringsAfterOffset(
    std::string* str,
    size_t start_offset,
    StringPiece find_this,
    StringPiece replace_with);
CRBASE_EXPORT void ReplaceSubstringsAfterOffset(
    std::u16string* str,
    size_t start_offset,
    StringPiece16 find_this,
    StringPiece16 replace_with);
CRBASE_EXPORT void ReplaceSubstringsAfterOffset(
    std::u32string* str,
    size_t start_offset,
    StringPiece32 find_this,
    StringPiece32 replace_with);
CRBASE_EXPORT void ReplaceSubstringsAfterOffset(
    std::wstring* str,
    size_t start_offset,
    WStringPiece find_this,
    WStringPiece replace_with);

// Reserves enough memory in |str| to accommodate |length_with_null| characters,
// sets the size of |str| to |length_with_null - 1| characters, and returns a
// pointer to the underlying contiguous array of characters.  This is typically
// used when calling a function that writes results into a character array, but
// the caller wants the data to be managed by a string-like object.  It is
// convenient in that is can be used inline in the call, and fast in that it
// avoids copying the results of the call from a char* into a string.
//
// Internally, this takes linear time because the resize() call 0-fills the
// underlying array for potentially all
// (|length_with_null - 1| * sizeof(string_type::value_type)) bytes.  Ideally we
// could avoid this aspect of the resize() call, as we expect the caller to
// immediately write over this memory, but there is no other way to set the size
// of the string, and not doing that will mean people who access |str| rather
// than str.c_str() will get back a string of whatever size |str| had on entry
// to this function (probably 0).
CRBASE_EXPORT char* WriteInto(std::string* str, size_t length_with_null);
CRBASE_EXPORT char16_t* WriteInto(std::u16string* str, size_t length_with_null);
CRBASE_EXPORT char32_t* WriteInto(std::u32string* str, size_t length_with_null);
CRBASE_EXPORT wchar_t* WriteInto(std::wstring* str, size_t length_with_null);

// Joins a list of strings into a single string, inserting |separator| (which
// may be empty) in between all elements.
//
// Note this is inverse of SplitString()/SplitStringPiece() defined in
// string_split.h.
//
// If possible, callers should build a vector of StringPieces and use the
// StringPiece variant, so that they do not create unnecessary copies of
// strings. For example, instead of using SplitString, modifying the vector,
// then using JoinString, use SplitStringPiece followed by JoinString so that no
// copies of those strings are created until the final join operation.
//
// Use StrCat (in crbase/strings/strcat.h) if you don't need a separator.
CRBASE_EXPORT std::string JoinString(Span<const std::string> parts,
                                     StringPiece separator);
CRBASE_EXPORT std::u16string JoinString(Span<const std::u16string> parts,
                                        StringPiece16 separator);
CRBASE_EXPORT std::u32string JoinString(Span<const std::u32string> parts,
                                        StringPiece32 separator);
CRBASE_EXPORT std::wstring JoinString(Span<const std::wstring> parts,
                                      WStringPiece separator);
CRBASE_EXPORT std::string JoinString(Span<const StringPiece> parts,
                                      StringPiece separator);
CRBASE_EXPORT std::u16string JoinString(Span<const StringPiece16> parts,
                                        StringPiece16 separator);
CRBASE_EXPORT std::u32string JoinString(Span<const StringPiece32> parts,
                                        StringPiece32 separator);
CRBASE_EXPORT std::wstring JoinString(Span<const WStringPiece> parts,
                                      WStringPiece separator);
// Explicit initializer_list overloads are required to break ambiguity when used
// with a literal initializer list (otherwise the compiler would not be able to
// decide between the string and StringPiece overloads).
CRBASE_EXPORT std::string JoinString(
    std::initializer_list<StringPiece> parts,
    StringPiece separator);
CRBASE_EXPORT std::u16string JoinString(
    std::initializer_list<StringPiece16> parts,
    StringPiece16 separator);
CRBASE_EXPORT std::u32string JoinString(
    std::initializer_list<StringPiece32> parts,
    StringPiece32 separator);
CRBASE_EXPORT std::wstring JoinString(
    std::initializer_list<WStringPiece> parts,
    WStringPiece separator);

// Replace $1-$2-$3..$9 in the format string with values from |subst|.
// Additionally, any number of consecutive '$' characters is replaced by that
// number less one. Eg $$->$, $$$->$$, etc. The offsets parameter here can be
// NULL. This only allows you to use up to nine replacements.
CRBASE_EXPORT std::string ReplaceStringPlaceholders(
    StringPiece format_string,
    const std::vector<std::string>& subst,
    std::vector<size_t>* offsets);
CRBASE_EXPORT std::u16string ReplaceStringPlaceholders(
    StringPiece16 format_string,
    const std::vector<std::u16string>& subst,
    std::vector<size_t>* offsets);
CRBASE_EXPORT std::u32string ReplaceStringPlaceholders(
    StringPiece32 format_string,
    const std::vector<std::u32string>& subst,
    std::vector<size_t>* offsets);
CRBASE_EXPORT std::wstring ReplaceStringPlaceholders(
    WStringPiece format_string,
    const std::vector<std::wstring>& subst,
    std::vector<size_t>* offsets);

// Single-string shortcut for ReplaceStringHolders. |offset| may be NULL.
CRBASE_EXPORT std::u16string ReplaceStringPlaceholders(
    const std::u16string& format_string,
    const std::u16string& a,
    size_t* offset);

CRBASE_EXPORT std::u32string ReplaceStringPlaceholders(
    const std::u32string& format_string,
    const std::u32string& a,
    size_t* offset);


#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)

// Utility functions to access the underlying string buffer as a wide char
// pointer.
//
// Note: These functions violate strict aliasing when char16_t and wchar_t are
// unrelated types. We thus pass -fno-strict-aliasing to the compiler on
// non-Windows platforms [1], and rely on it being off in Clang's CL mode [2].
//
// [1] https://crrev.com/b9a0976622/build/config/compiler/BUILD.gn#244
// [2]
// https://github.com/llvm/llvm-project/blob/1e28a66/clang/lib/Driver/ToolChains/Clang.cpp#L3949
inline wchar_t* as_writable_wcstr(char16_t* str) {
  return reinterpret_cast<wchar_t*>(str);
}

inline wchar_t* as_writable_wcstr(std::u16string& str) {
  return reinterpret_cast<wchar_t*>(data(str));
}

inline const wchar_t* as_wcstr(const char16_t* str) {
  return reinterpret_cast<const wchar_t*>(str);
}

inline const wchar_t* as_wcstr(StringPiece16 str) {
  return reinterpret_cast<const wchar_t*>(str.data());
}

// Utility functions to access the underlying string buffer as a char16_t
// pointer.
inline char16_t* as_writable_u16cstr(wchar_t* str) {
  return reinterpret_cast<char16_t*>(str);
}

inline char16_t* as_writable_u16cstr(std::wstring& str) {
  return reinterpret_cast<char16_t*>(data(str));
}

inline const char16_t* as_u16cstr(const wchar_t* str) {
  return reinterpret_cast<const char16_t*>(str);
}

inline const char16_t* as_u16cstr(WStringPiece str) {
  return reinterpret_cast<const char16_t*>(str.data());
}

// Utility functions to convert between cr::WStringPiece and
// cr::StringPiece16.
inline WStringPiece AsWStringPiece(StringPiece16 str) {
  return WStringPiece(as_wcstr(str.data()), str.size());
}

inline StringPiece16 AsStringPiece16(WStringPiece str) {
  return StringPiece16(as_u16cstr(str.data()), str.size());
}

inline std::wstring AsWString(StringPiece16 str) {
  return std::wstring(as_wcstr(str.data()), str.size());
}

inline std::u16string AsString16(WStringPiece str) {
  return std::u16string(as_u16cstr(str.data()), str.size());
}

#elif defined(MINI_CHROMIUM_WCHAR_T_IS_UTF32)

// Utility functions to access the underlying string buffer as a wide char
// pointer.
//
// Note: These functions violate strict aliasing when char16_t and wchar_t are
// unrelated types. We thus pass -fno-strict-aliasing to the compiler on
// non-Windows platforms [1], and rely on it being off in Clang's CL mode [2].
//
// [1] https://crrev.com/b9a0976622/build/config/compiler/BUILD.gn#244
// [2]
// https://github.com/llvm/llvm-project/blob/1e28a66/clang/lib/Driver/ToolChains/Clang.cpp#L3949
inline wchar_t* as_writable_wcstr(char32_t* str) {
  return reinterpret_cast<wchar_t*>(str);
}

inline wchar_t* as_writable_wcstr(std::u32string& str) {
  return reinterpret_cast<wchar_t*>(data(str));
}

inline const wchar_t* as_wcstr(const char32_t* str) {
  return reinterpret_cast<const wchar_t*>(str);
}

inline const wchar_t* as_wcstr(StringPiece32 str) {
  return reinterpret_cast<const wchar_t*>(str.data());
}

// Utility functions to access the underlying string buffer as a char16_t
// pointer.
inline char32_t* as_writable_u32cstr(wchar_t* str) {
  return reinterpret_cast<char16_t*>(str);
}

inline char32_t* as_writable_u32cstr(std::wstring& str) {
  return reinterpret_cast<char32_t*>(data(str));
}

inline const char32_t* as_u32cstr(const wchar_t* str) {
  return reinterpret_cast<const char32_t*>(str);
}

inline const char32_t* as_u32cstr(WStringPiece str) {
  return reinterpret_cast<const char16_t*>(str.data());
}

// Utility functions to convert between cr::WStringPiece and
// cr::StringPiece16.
inline WStringPiece AsWStringPiece(StringPiece32 str) {
  return WStringPiece(as_wcstr(str.data()), str.size());
}

inline StringPiece32 AsStringPiece32(WStringPiece str) {
  return StringPiece16(as_u16cstr(str.data()), str.size());
}

inline std::wstring AsWString(StringPiece32 str) {
  return std::wstring(as_wcstr(str.data()), str.size());
}

inline std::u32string AsString32(WStringPiece str) {
  return std::u32string(as_u32cstr(str.data()), str.size());
}

#endif

}  // namespace cr

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/strings/win/string_util_win.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include "crbase/strings/posix/string_util_posix.h"
#else
#error Define string operations appropriately for your platform
#endif

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_H_

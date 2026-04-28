// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_ESCAPE_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_ESCAPE_H_

#include <stdint.h>

#include <set>
#include <string>

#include "cr_base/base_export.h"
#include "cr_base/strings/utf_offset_string_conversions.h"
#include "cr_build/build_config.h"

namespace cr {

// Escaping --------------------------------------------------------------------

// Escapes all characters except unreserved characters. Unreserved characters,
// as defined in RFC 3986, include alphanumerics and -._~
CRBASE_EXPORT std::string EscapeAllExceptUnreserved(cr::StringPiece text);

// Escapes characters in text suitable for use as a query parameter value.
// We %XX everything except alphanumerics and -_.!~*'()
// Spaces change to "+" unless you pass usePlus=false.
// This is basically the same as encodeURIComponent in javascript.
CRBASE_EXPORT std::string EscapeQueryParamValue(cr::StringPiece text,
                                                bool use_plus);

// Escapes a partial or complete file/pathname.  This includes:
// non-printable, non-7bit, and (including space)  "#%:<>?[\]^`{|}
CRBASE_EXPORT std::string EscapePath(cr::StringPiece path);

// Escapes application/x-www-form-urlencoded content.  This includes:
// non-printable, non-7bit, and (including space)  ?>=<;+'&%$#"![\]^`{|}
// Space is escaped as + (if use_plus is true) and other special characters
// as %XX (hex).
CRBASE_EXPORT std::string EscapeUrlEncodedData(cr::StringPiece path,
                                               bool use_plus);

// Escapes all non-ASCII input, as well as escaping % to %25.
CRBASE_EXPORT std::string EscapeNonASCIIAndPercent(cr::StringPiece input);

// Escapes all non-ASCII input. Note this function leaves % unescaped, which
// means the unescaping the resulting string will not give back the original
// input.
CRBASE_EXPORT std::string EscapeNonASCII(cr::StringPiece input);

// Escapes characters in text suitable for use as an external protocol handler
// command.
// We %XX everything except alphanumerics and -_.!~*'() and the restricted
// characters (;/?:@&=+$,#[]) and a valid percent escape sequence (%XX).
CRBASE_EXPORT std::string EscapeExternalHandlerValue(cr::StringPiece text);

// Appends the given character to the output string, escaping the character if
// the character would be interpreted as an HTML delimiter.
CRBASE_EXPORT void AppendEscapedCharForHTML(char c, std::string* output);

// Escapes chars that might cause this text to be interpreted as HTML tags.
CRBASE_EXPORT std::string EscapeForHTML(cr::StringPiece text);
CRBASE_EXPORT std::u16string EscapeForHTML(cr::StringPiece16 text);

// Unescaping ------------------------------------------------------------------

class UnescapeRule {
 public:
  // A combination of the following flags that is passed to the unescaping
  // functions.
  typedef uint32_t Type;

  // Don't unescape anything at all.
  static constexpr Type NONE = 0;

  // Don't unescape anything special, but all normal unescaping will happen.
  // This is a placeholder and can't be combined with other flags (since it's
  // just the absence of them). All other unescape rules imply "normal" in
  // addition to their special meaning. Things like escaped letters, digits,
  // and most symbols will get unescaped with this mode.
  static constexpr Type NORMAL = 1 << 0;

  // Convert %20 to spaces. In some places where we're showing URLs, we may
  // want this. In places where the URL may be copied and pasted out, then
  // you wouldn't want this since it might not be interpreted in one piece
  // by other applications.  Other UTF-8 spaces will not be unescaped.
  static constexpr Type SPACES = 1 << 1;

  // Unescapes '/' and '\\'. If these characters were unescaped, the resulting
  // URL won't be the same as the source one. Moreover, they are dangerous to
  // unescape in strings that will be used as file paths or names. This value
  // should only be used when slashes don't have special meaning, like data
  // URLs.
  static constexpr Type PATH_SEPARATORS = 1 << 2;

  // Unescapes various characters that will change the meaning of URLs,
  // including '%', '+', '&', '#'. Does not unescape path separators.
  // If these characters were unescaped, the resulting URL won't be the same
  // as the source one. This flag is used when generating final output like
  // filenames for URLs where we won't be interpreting as a URL and want to do
  // as much unescaping as possible.
  static constexpr Type URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS = 1 << 3;

  // URL queries use "+" for space. This flag controls that replacement.
  static constexpr Type REPLACE_PLUS_WITH_SPACE = 1 << 4;
};

// Unescapes |escaped_text| and returns the result.
// Unescaping consists of looking for the exact pattern "%XX", where each X is
// a hex digit, and converting to the character with the numerical value of
// those digits. Thus "i%20=%203%3b" unescapes to "i = 3;", if the
// "UnescapeRule::SPACES" used.
//
// This method does not ensure that the output is a valid string using any
// character encoding. However, it does leave escaped certain byte sequences
// that would be dangerous to display to the user, because if interpreted as
// UTF-8, they could be used to mislead the user. Callers that want to
// unconditionally unescape everything for uses other than displaying data to
// the user should use UnescapeBinaryURLComponent().
CRBASE_EXPORT std::string UnescapeURLComponent(cr::StringPiece escaped_text,
                                               UnescapeRule::Type rules);

// Unescapes the given substring as a URL, and then tries to interpret the
// result as being encoded as UTF-8. If the result is convertible into UTF-8, it
// will be returned as converted. If it is not, the original escaped string will
// be converted into a std::u16string and returned.  |adjustments| provides
// information on how the original string was adjusted to get the string
// returned.
CRBASE_EXPORT std::u16string UnescapeAndDecodeUTF8URLComponentWithAdjustments(
  cr::StringPiece text,
    UnescapeRule::Type rules,
    OffsetAdjuster::Adjustments* adjustments);

// Unescapes a component of a URL for use as binary data. Unlike
// UnescapeURLComponent, leaves nothing unescaped, including nulls, invalid
// characters, characters that are unsafe to display, etc. This should *not*
// be used when displaying the decoded data to the user.
//
// Only the NORMAL and REPLACE_PLUS_WITH_SPACE rules are allowed.
CRBASE_EXPORT std::string UnescapeBinaryURLComponent(
    cr::StringPiece escaped_text,
    UnescapeRule::Type rules = UnescapeRule::NORMAL);

// Variant of UnescapeBinaryURLComponent().  Writes output to |unescaped_text|.
// Returns true on success, returns false and clears |unescaped_text| on
// failure. Fails on characters escaped that are unsafe to unescape in some
// contexts, which are defined as characters "\0" through "\x1F" (Which includes
// CRLF but not space), and optionally path separators. Path separators include
// both forward and backward slashes on all platforms. Does not fail if any of
// those characters appear unescaped in the input string.
CRBASE_EXPORT bool UnescapeBinaryURLComponentSafe(cr::StringPiece escaped_text,
                                                  bool fail_on_path_separators,
                                                  std::string* unescaped_text);

// Returns true if |escaped_text| contains any element of |bytes| in
// percent-encoded form.
//
// For example, if |bytes| is {'%', '/'}, returns true if |escaped_text|
// contains "%25" or "%2F", but not if it just contains bare '%' or '/'
// characters.
CRBASE_EXPORT bool ContainsEncodedBytes(cr::StringPiece escaped_text,
                                        const std::set<unsigned char>& bytes);

// Unescapes the following ampersand character codes from |text|:
// &lt; &gt; &amp; &quot; &#39;
CRBASE_EXPORT std::u16string UnescapeForHTML(cr::StringPiece16 text);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_ESCAPE_H_
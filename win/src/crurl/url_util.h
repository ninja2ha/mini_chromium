// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRURL_URL_UTIL_H_
#define MINI_CHROMIUM_SRC_CRURL_URL_UTIL_H_

#include <string>

#include "crbase/strings/string16.h"
#include "crurl/third_party/mozilla/url_parse.h"
#include "crurl/url_canon.h"
#include "crurl/url_constants.h"
#include "crurl/url_export.h"

namespace crurl {

// Init ------------------------------------------------------------------------

// Initialization is NOT required, it will be implicitly initialized when first
// used. However, this implicit initialization is NOT threadsafe. If you are
// using this library in a threaded environment and don't have a consistent
// "first call" (an example might be calling AddStandardScheme with your special
// application-specific schemes) then you will want to call initialize before
// spawning any threads.
//
// It is OK to call this function more than once, subsequent calls will be
// no-ops, unless Shutdown was called in the mean time. This will also be a
// no-op if other calls to the library have forced an initialization beforehand.
CRURL_EXPORT void Initialize();

// Cleanup is not required, except some strings may leak. For most user
// applications, this is fine. If you're using it in a library that may get
// loaded and unloaded, you'll want to unload to properly clean up your
// library.
CRURL_EXPORT void Shutdown();

// Schemes --------------------------------------------------------------------

// Types of a scheme representing the requirements on the data represented by
// the authority component of a URL with the scheme.
enum SchemeType {
  // The authority component of a URL with the scheme, if any, has the port
  // (the default values may be omitted in a serialization).
  SCHEME_WITH_PORT,
  // The authority component of a URL with the scheme, if any, doesn't have a
  // port.
  SCHEME_WITHOUT_PORT,
  // A URL with the scheme doesn't have the authority component.
  SCHEME_WITHOUT_AUTHORITY,
};

// A pair for representing a standard scheme name and the SchemeType for it.
struct CRURL_EXPORT SchemeWithType {
  const char* scheme;
  SchemeType type;
};

// Adds an application-defined scheme to the internal list of "standard-format"
// URL schemes. A standard-format scheme adheres to what RFC 3986 calls "generic
// URI syntax" (https://tools.ietf.org/html/rfc3986#section-3).
//
// This function is not threadsafe and can not be called concurrently with any
// other url_util function. It will assert if the list of standard schemes has
// been locked (see LockStandardSchemes).
CRURL_EXPORT void AddStandardScheme(const char* new_scheme,
                                    SchemeType scheme_type);

// Sets a flag to prevent future calls to AddStandardScheme from succeeding.
//
// This is designed to help prevent errors for multithreaded applications.
// Normal usage would be to call AddStandardScheme for your custom schemes at
// the beginning of program initialization, and then LockStandardSchemes. This
// prevents future callers from mistakenly calling AddStandardScheme when the
// program is running with multiple threads, where such usage would be
// dangerous.
//
// We could have had AddStandardScheme use a lock instead, but that would add
// some platform-specific dependencies we don't otherwise have now, and is
// overkill considering the normal usage is so simple.
CRURL_EXPORT void LockStandardSchemes();

// Locates the scheme in the given string and places it into |found_scheme|,
// which may be NULL to indicate the caller does not care about the range.
//
// Returns whether the given |compare| scheme matches the scheme found in the
// input (if any). The |compare| scheme must be a valid canonical scheme or
// the result of the comparison is undefined.
CRURL_EXPORT bool FindAndCompareScheme(const char* str,
                                       int str_len,
                                       const char* compare,
                                       Component* found_scheme);
CRURL_EXPORT bool FindAndCompareScheme(const cr::char16* str,
                                       int str_len,
                                       const char* compare,
                                       Component* found_scheme);
inline bool FindAndCompareScheme(const std::string& str,
                                 const char* compare,
                                 Component* found_scheme) {
  return FindAndCompareScheme(str.data(), static_cast<int>(str.size()),
                              compare, found_scheme);
}
inline bool FindAndCompareScheme(const cr::string16& str,
                                 const char* compare,
                                 Component* found_scheme) {
  return FindAndCompareScheme(str.data(), static_cast<int>(str.size()),
                              compare, found_scheme);
}

// Returns true if the given scheme identified by |scheme| within |spec| is in
// the list of known standard-format schemes (see AddStandardScheme).
CRURL_EXPORT bool IsStandard(const char* spec, const Component& scheme);
CRURL_EXPORT bool IsStandard(const cr::char16* spec, const Component& scheme);

// Returns true and sets |type| to the SchemeType of the given scheme
// identified by |scheme| within |spec| if the scheme is in the list of known
// standard-format schemes (see AddStandardScheme).
CRURL_EXPORT bool GetStandardSchemeType(const char* spec,
                                        const Component& scheme,
                                        SchemeType* type);

// URL library wrappers -------------------------------------------------------

// Parses the given spec according to the extracted scheme type. Normal users
// should use the URL object, although this may be useful if performance is
// critical and you don't want to do the heap allocation for the std::string.
//
// As with the Canonicalize* functions, the charset converter can
// be NULL to use UTF-8 (it will be faster in this case).
//
// Returns true if a valid URL was produced, false if not. On failure, the
// output and parsed structures will still be filled and will be consistent,
// but they will not represent a loadable URL.
CRURL_EXPORT bool Canonicalize(const char* spec,
                               int spec_len,
                               bool trim_path_end,
                               CharsetConverter* charset_converter,
                               CanonOutput* output,
                               Parsed* output_parsed);
CRURL_EXPORT bool Canonicalize(const cr::char16* spec,
                               int spec_len,
                               bool trim_path_end,
                               CharsetConverter* charset_converter,
                               CanonOutput* output,
                               Parsed* output_parsed);

// Resolves a potentially relative URL relative to the given parsed base URL.
// The base MUST be valid. The resulting canonical URL and parsed information
// will be placed in to the given out variables.
//
// The relative need not be relative. If we discover that it's absolute, this
// will produce a canonical version of that URL. See Canonicalize() for more
// about the charset_converter.
//
// Returns true if the output is valid, false if the input could not produce
// a valid URL.
CRURL_EXPORT bool ResolveRelative(const char* base_spec,
                                  int base_spec_len,
                                  const Parsed& base_parsed,
                                  const char* relative,
                                  int relative_length,
                                  CharsetConverter* charset_converter,
                                  CanonOutput* output,
                                  Parsed* output_parsed);
CRURL_EXPORT bool ResolveRelative(const char* base_spec,
                                  int base_spec_len,
                                  const Parsed& base_parsed,
                                  const cr::char16* relative,
                                  int relative_length,
                                  CharsetConverter* charset_converter,
                                  CanonOutput* output,
                                  Parsed* output_parsed);

// Replaces components in the given VALID input URL. The new canonical URL info
// is written to output and out_parsed.
//
// Returns true if the resulting URL is valid.
CRURL_EXPORT bool ReplaceComponents(const char* spec,
                                    int spec_len,
                                    const Parsed& parsed,
                                    const Replacements<char>& replacements,
                                    CharsetConverter* charset_converter,
                                    CanonOutput* output,
                                    Parsed* out_parsed);
CRURL_EXPORT bool ReplaceComponents(
    const char* spec,
    int spec_len,
    const Parsed& parsed,
    const Replacements<cr::char16>& replacements,
    CharsetConverter* charset_converter,
    CanonOutput* output,
    Parsed* out_parsed);

// String helper functions ----------------------------------------------------

// Unescapes the given string using URL escaping rules.
CRURL_EXPORT void DecodeURLEscapeSequences(const char* input,
                                           int length,
                                           CanonOutputW* output);

// Escapes the given string as defined by the JS method encodeURIComponent. See
// https://developer.mozilla.org/en/JavaScript/Reference/Global_Objects/encodeURIComponent
CRURL_EXPORT void EncodeURIComponent(const char* input,
                                     int length,
                                     CanonOutput* output);

}  // namespace crurl

#endif  // MINI_CHROMIUM_SRC_CRURL_URL_UTIL_H_
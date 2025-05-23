// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING16_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING16_H_

// WHAT:
// A version of std::basic_string that provides 2-byte characters even when
// wchar_t is not implemented as a 2-byte type. You can access this class as
// string16. We also define char16, which string16 is based upon.
//
// WHY:
// On Windows, wchar_t is 2 bytes, and it can conveniently handle UTF-16/UCS-2
// data. Plenty of existing code operates on strings encoded as UTF-16.
//
// On many other platforms, sizeof(wchar_t) is 4 bytes by default. We can make
// it 2 bytes by using the GCC flag -fshort-wchar. But then std::wstring fails
// at run time, because it calls some functions (like wcslen) that come from
// the system's native C library -- which was built with a 4-byte wchar_t!
// It's wasteful to use 4-byte wchar_t strings to carry UTF-16 data, and it's
// entirely improper on those systems where the encoding of wchar_t is defined
// as UTF-32.
//
// Here, we define string16, which is similar to std::wstring but replaces all
// libc functions with custom, 2-byte-char compatible routines. It is capable
// of carrying UTF-16-encoded data.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <functional>
#include <string>

#include "crbase/base_export.h"
#include "crbase/build_platform.h"

#if defined(CR_WCHAR_T_IS_UTF16)

// Define a macro for wrapping construction of char16 arrays and string16s from
// a literal string. This indirection allows for an easier migration of
// base::char16 to char16_t on platforms where WCHAR_T_IS_UTF16, as only a one
// character change to the macro will be necessary.
// This macro does not exist when WCHAR_T_IS_UTF32, as it is currently not
// possible to create a char array form a literal in this case.
// TODO(https://crbug.com/911896): Remove this macro once base::char16 is
// char16_t on all platforms.
#define CR_STRING16_LITERAL(x) L##x

namespace cr {

typedef wchar_t char16;
typedef std::wstring string16;

}  // namespace cr

#elif defined(CR_WCHAR_T_IS_UTF32)

#include <wchar.h>  // for mbstate_t

namespace cr {

typedef uint16_t char16;

// char16 versions of the functions required by string16_char_traits; these
// are based on the wide character functions of similar names ("w" or "wcs"
// instead of "c16").
CRBASE_EXPORT int c16memcmp(const char16* s1, const char16* s2, size_t n);
CRBASE_EXPORT size_t c16len(const char16* s);
CRBASE_EXPORT const char16* c16memchr(const char16* s, char16 c, size_t n);
CRBASE_EXPORT char16* c16memmove(char16* s1, const char16* s2, size_t n);
CRBASE_EXPORT char16* c16memcpy(char16* s1, const char16* s2, size_t n);
CRBASE_EXPORT char16* c16memset(char16* s, char16 c, size_t n);

// This namespace contains the implementation of base::string16 along with
// things that need to be found via argument-dependent lookup from a
// base::string16.
namespace string16_internals {

struct string16_char_traits {
  typedef char16 char_type;
  typedef int int_type;

  // int_type needs to be able to hold each possible value of char_type, and in
  // addition, the distinct value of eof().
  static_assert(sizeof(int_type) > sizeof(char_type),
                "int must be larger than 16 bits wide");

  typedef std::streamoff off_type;
  typedef mbstate_t state_type;
  typedef std::fpos<state_type> pos_type;

  static void assign(char_type& c1, const char_type& c2) {
    c1 = c2;
  }

  static bool eq(const char_type& c1, const char_type& c2) {
    return c1 == c2;
  }
  static bool lt(const char_type& c1, const char_type& c2) {
    return c1 < c2;
  }

  static int compare(const char_type* s1, const char_type* s2, size_t n) {
    return c16memcmp(s1, s2, n);
  }

  static size_t length(const char_type* s) {
    return c16len(s);
  }

  static const char_type* find(const char_type* s, size_t n,
                               const char_type& a) {
    return c16memchr(s, a, n);
  }

  static char_type* move(char_type* s1, const char_type* s2, size_t n) {
    return c16memmove(s1, s2, n);
  }

  static char_type* copy(char_type* s1, const char_type* s2, size_t n) {
    return c16memcpy(s1, s2, n);
  }

  static char_type* assign(char_type* s, size_t n, char_type a) {
    return c16memset(s, a, n);
  }

  static int_type not_eof(const int_type& c) {
    return eq_int_type(c, eof()) ? 0 : c;
  }

  static char_type to_char_type(const int_type& c) {
    return char_type(c);
  }

  static int_type to_int_type(const char_type& c) {
    return int_type(c);
  }

  static bool eq_int_type(const int_type& c1, const int_type& c2) {
    return c1 == c2;
  }

  static int_type eof() {
    return static_cast<int_type>(EOF);
  }
};

}  // namespace string16_internals

typedef std::basic_string<char16,
                          cr::string16_internals::string16_char_traits>
    string16;

namespace string16_internals {

CRBASE_EXPORT extern std::ostream& operator<<(std::ostream& out,
                                              const string16& str);

}  // namespace string16_internals

}  // namespace cr

// The string class will be explicitly instantiated only once, in string16.cc.
//
// std::basic_string<> in GNU libstdc++ contains a static data member,
// _S_empty_rep_storage, to represent empty strings.  When an operation such
// as assignment or destruction is performed on a string, causing its existing
// data member to be invalidated, it must not be freed if this static data
// member is being used.  Otherwise, it counts as an attempt to free static
// (and not allocated) data, which is a memory error.
//
// Generally, due to C++ template magic, _S_empty_rep_storage will be marked
// as a coalesced symbol, meaning that the linker will combine multiple
// instances into a single one when generating output.
//
// If a string class is used by multiple shared libraries, a problem occurs.
// Each library will get its own copy of _S_empty_rep_storage.  When strings
// are passed across a library boundary for alteration or destruction, memory
// errors will result.  GNU libstdc++ contains a configuration option,
// --enable-fully-dynamic-string (_GLIBCXX_FULLY_DYNAMIC_STRING), which
// disables the static data member optimization, but it's a good optimization
// and non-STL code is generally at the mercy of the system's STL
// configuration.  Fully-dynamic strings are not the default for GNU libstdc++
// libstdc++ itself or for the libstdc++ installations on the systems we care
// about, such as Mac OS X and relevant flavors of Linux.
//
// See also http://gcc.gnu.org/bugzilla/show_bug.cgi?id=24196 .
//
// To avoid problems, string classes need to be explicitly instantiated only
// once, in exactly one library.  All other string users see it via an "extern"
// declaration.  This is precisely how GNU libstdc++ handles
// std::basic_string<char> (string) and std::basic_string<wchar_t> (wstring).
//
// This also works around a Mac OS X linker bug in ld64-85.2.1 (Xcode 3.1.2),
// in which the linker does not fully coalesce symbols when dead code
// stripping is enabled.  This bug causes the memory errors described above
// to occur even when a std::basic_string<> does not cross shared library
// boundaries, such as in statically-linked executables.
//
// TODO(mark): File this bug with Apple and update this note with a bug number.

extern template class CRBASE_EXPORT
    std::basic_string<cr::char16,
                      cr::string16_internals::string16_char_traits>;

// Specialize std::hash for base::string16. Although the style guide forbids
// this in general, it is necessary for consistency with WCHAR_T_IS_UTF16
// platforms, where base::string16 is a type alias for std::wstring.
namespace std {
template <>
struct hash<cr::string16> {
  std::size_t operator()(const cr::string16& s) const {
    std::size_t result = 0;
    for (cr::char16 c : s)
      result = (result * 131) + c;
    return result;
  }
};
}  // namespace std

#endif  // CR_WCHAR_T_IS_UTF32

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING16_H_
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_WIN_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_WIN_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "crbase/logging.h"

namespace cr {

// Chromium code style is to not use malloc'd strings; this is only for use
// for interaction with APIs that require it.
inline char* strdup(const char* str) {
  return _strdup(str);
}

inline int vsnprintf(char* buffer, size_t size,
                     const char* format, va_list arguments) {
  int length = vsnprintf_s(buffer, size, size - 1, format, arguments);
  if (length < 0)
    return _vscprintf(format, arguments);
  return length;
}

inline int vswprintf(wchar_t* buffer, size_t size,
                     const wchar_t* format, va_list arguments) {
  CR_DCHECK(IsWprintfFormatPortable(format));

  int length = _vsnwprintf_s(buffer, size, size - 1, format, arguments);
  if (length < 0)
    return _vscwprintf(format, arguments);
  return length;
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_WIN_H_
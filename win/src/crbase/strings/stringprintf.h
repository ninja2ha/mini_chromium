// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRINGPRINTF_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRINGPRINTF_H_

#include <stdarg.h>   // va_list

#include <string>

#include "crbase/base_export.h"
#include "crbase/compiler_specific.h"
#include "crbase/build_platform.h"

namespace cr {

// Return a C++ string given printf-like input.
CRBASE_EXPORT std::string StringPrintf(const char* format, ...)
    CR_PRINTF_FORMAT(1, 2) CR_WARN_UNUSED_RESULT;
#if defined(MINI_CHROMIUM_OS_WIN)
// Note: Unfortunately compile time checking of the format string for UTF-16
// strings is not supported by any compiler, thus these functions should be used
// carefully and sparingly. Also applies to SStringPrintf and StringAppendV
// below.
CRBASE_EXPORT std::wstring StringPrintf(const wchar_t* format, ...)
    CR_WPRINTF_FORMAT(1, 2) CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::u16string StringPrintf(const char16_t* format, ...)
    CR_WPRINTF_FORMAT(1, 2) CR_WARN_UNUSED_RESULT;
#endif

// Return a C++ string given vprintf-like input.
CRBASE_EXPORT std::string StringPrintV(const char* format, va_list ap)
    CR_PRINTF_FORMAT(1, 0) CR_WARN_UNUSED_RESULT;

// Store result into a supplied string and return it.
CRBASE_EXPORT const std::string& SStringPrintf(std::string* dst,
                                               const char* format,
                                               ...) CR_PRINTF_FORMAT(2, 3);
#if defined(MINI_CHROMIUM_OS_WIN)
CRBASE_EXPORT const std::wstring& SStringPrintf(std::wstring* dst,
                                                const wchar_t* format,
                                                ...) CR_WPRINTF_FORMAT(2, 3);
CRBASE_EXPORT const std::u16string& SStringPrintf(std::u16string* dst,
                                                  const char16_t* format,
                                                  ...) CR_WPRINTF_FORMAT(2, 3);
#endif

// Append result to a supplied string.
CRBASE_EXPORT void StringAppendF(std::string* dst, const char* format, ...)
    CR_PRINTF_FORMAT(2, 3);
#if defined(MINI_CHROMIUM_OS_WIN)
CRBASE_EXPORT void StringAppendF(std::wstring* dst, const wchar_t* format, ...)
    CR_WPRINTF_FORMAT(2, 3);
CRBASE_EXPORT void StringAppendF(std::u16string* dst, 
                                 const char16_t* format, ...)
    CR_WPRINTF_FORMAT(2, 3);
#endif

// Lower-level routine that takes a va_list and appends to a specified
// string.  All other routines are just convenience wrappers around it.
CRBASE_EXPORT void StringAppendV(std::string* dst, 
                                 const char* format, va_list ap)
    CR_PRINTF_FORMAT(2, 0);
#if defined(MINI_CHROMIUM_OS_WIN)
CRBASE_EXPORT void StringAppendV(std::wstring* dst,
                                 const wchar_t* format,
                                 va_list ap) CR_WPRINTF_FORMAT(2, 0);
CRBASE_EXPORT void StringAppendV(std::u16string* dst,
                                 const char16_t* format,
                                 va_list ap) CR_WPRINTF_FORMAT(2, 0);
#endif

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRINGPRINTF_H_
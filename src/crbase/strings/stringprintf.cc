// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/strings/stringprintf.h"

#include <errno.h>
#include <stddef.h>

#include <vector>

#include "crbase/logging/logging.h"
#include "crbase/helper/scoped_clear_last_error.h"
#include "crbase/stl_util.h"
#include "crbase/strings/string_util.h"
#include "crbuild/build_config.h"

namespace cr {

namespace {

// Overloaded wrappers around vsnprintf and vswprintf. The buf_size parameter
// is the size of the buffer. These return the number of characters in the
// formatted string excluding the NUL terminator. If the buffer is not
// large enough to accommodate the formatted string without truncation, they
// return the number of characters that would be in the fully-formatted string
// (vsnprintf, and vswprintf on Windows), or -1 (vswprintf on POSIX platforms).
inline int vsnprintfT(char* buffer,
                      size_t buf_size,
                      const char* format,
                      va_list argptr) {
  return cr::vsnprintf(buffer, buf_size, format, argptr);
}

#if defined(MINI_CHROMIUM_OS_WIN)
inline int vsnprintfT(char16_t* buffer,
                      size_t buf_size,
                      const char16_t* format,
                      va_list argptr) {
  return cr::vswprintf(reinterpret_cast<wchar_t*>(buffer), buf_size,
                       reinterpret_cast<const wchar_t*>(format), argptr);
}
inline int vsnprintfT(wchar_t* buffer,
                      size_t buf_size,
                      const wchar_t* format,
                      va_list argptr) {
  return cr::vswprintf(buffer, buf_size, format, argptr);
}
#endif

// Templatized backend for StringPrintF/StringAppendF. This does not finalize
// the va_list, the caller is expected to do that.
template <class CharT>
static void StringAppendVT(std::basic_string<CharT>* dst,
                           const CharT* format,
                           va_list ap) {
  // First try with a small fixed size buffer.
  // This buffer size should be kept in sync with StringUtilTest.GrowBoundary
  // and StringUtilTest.StringPrintfBounds.
  CharT stack_buf[1024];

  va_list ap_copy;
  va_copy(ap_copy, ap);

  cr::ScopedClearLastError last_error;
  int result = vsnprintfT(stack_buf, cr::size(stack_buf), format, ap_copy);
  va_end(ap_copy);

  if (result >= 0 && result < static_cast<int>(cr::size(stack_buf))) {
    // It fit.
    dst->append(stack_buf, result);
    return;
  }

  // Repeatedly increase buffer size until it fits.
  int mem_length = static_cast<int>(cr::size(stack_buf));
  while (true) {
    if (result < 0) {
#if defined(MINI_CHROMIUM_OS_WIN)
      // On Windows, vsnprintfT always returns the number of characters in a
      // fully-formatted string, so if we reach this point, something else is
      // wrong and no amount of buffer-doubling is going to fix it.
      return;
#else
      if (errno != 0 && errno != EOVERFLOW)
        return;
      // Try doubling the buffer size.
      mem_length *= 2;
#endif
    } else {
      // We need exactly "result + 1" characters.
      mem_length = result + 1;
    }

    if (mem_length > 32 * 1024 * 1024) {
      // That should be plenty, don't try anything larger.  This protects
      // against huge allocations when using vsnprintfT implementations that
      // return -1 for reasons other than overflow without setting errno.
      CR_DLOG(Warning) << "Unable to printf the requested string due to size.";
      return;
    }

    std::vector<CharT> mem_buf(mem_length);

    // NOTE: You can only use a va_list once.  Since we're in a while loop, we
    // need to make a new copy each time so we don't use up the original.
    va_copy(ap_copy, ap);
    result = vsnprintfT(&mem_buf[0], mem_length, format, ap_copy);
    va_end(ap_copy);

    if ((result >= 0) && (result < mem_length)) {
      // It fit.
      dst->append(&mem_buf[0], result);
      return;
    }
  }
}

}  // namespace

std::string StringPrintf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  std::string result;
  StringAppendV(&result, format, ap);
  va_end(ap);
  return result;
}

#if defined(MINI_CHROMIUM_OS_WIN)
std::u16string StringPrintf(const char16_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  std::u16string result;
  StringAppendV(&result, format, ap);
  va_end(ap);
  return result;
}

std::wstring StringPrintf(const wchar_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  std::wstring result;
  StringAppendV(&result, format, ap);
  va_end(ap);
  return result;
}
#endif

std::string StringPrintV(const char* format, va_list ap) {
  std::string result;
  StringAppendV(&result, format, ap);
  return result;
}

const std::string& SStringPrintf(std::string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  dst->clear();
  StringAppendV(dst, format, ap);
  va_end(ap);
  return *dst;
}

#if defined(MINI_CHROMIUM_OS_WIN)
const std::u16string& SStringPrintf(std::u16string* dst,
                                    const char16_t* format,
                                    ...) {
  va_list ap;
  va_start(ap, format);
  dst->clear();
  StringAppendV(dst, format, ap);
  va_end(ap);
  return *dst;
}

const std::wstring& SStringPrintf(std::wstring* dst,
                                  const wchar_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  dst->clear();
  StringAppendV(dst, format, ap);
  va_end(ap);
  return *dst;
}
#endif

void StringAppendF(std::string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

#if defined(MINI_CHROMIUM_OS_WIN)
void StringAppendF(std::u16string* dst, const char16_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

void StringAppendF(std::wstring* dst, const wchar_t* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}
#endif

void StringAppendV(std::string* dst, const char* format, va_list ap) {
  StringAppendVT(dst, format, ap);
}

#if defined(MINI_CHROMIUM_OS_WIN)
void StringAppendV(std::u16string* dst, const char16_t* format, va_list ap) {
  StringAppendVT(dst, format, ap);
}

void StringAppendV(std::wstring* dst, const wchar_t* format, va_list ap) {
  StringAppendVT(dst, format, ap);
}
#endif

}  // namespace cr

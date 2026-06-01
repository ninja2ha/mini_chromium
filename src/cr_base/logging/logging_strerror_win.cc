// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/logging/logging_strerror.h"

#include <stdio.h>

// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>

#include <type_traits>

namespace cr {
namespace logging {

namespace {

template<typename CharType>
std::basic_string<wchar_t> SystemErrorCodeToHexString(SystemErrorCode error) {
  static_assert(std::is_integral<SystemErrorCode>::value, "");

  constexpr int kPrefixLength = 2;
  std::basic_string<CharType> result(sizeof(error) + kPrefixLength, '\0');
  result[0] = '0';
  result[1] = 'x';

  constexpr int kErrorSize = sizeof(error);
  for (int i = kErrorSize - 1; i > 0; i--) {
    int chr = error & 0xF;
    result[kPrefixLength + i] = static_cast<CharType>(
        (chr > 9 ?  (chr - 10 + 'A') : (chr + '0')));
    error >>= 4;
  }

  return result;
}

// Copied from chromium/base/strings/sys_string_conversions_win.cc
std::string WideToUTF8(const std::wstring& wide) {
  int wide_length = static_cast<int>(wide.length());
  if (wide_length == 0) {
    return std::string();
  }

  // Compute the length of the buffer we'll need.
  int charcount = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide_length,
                                      NULL, 0, NULL, NULL);
  if (charcount <= 0) {
    return std::string();
  }

  std::string mb;
  mb.resize(static_cast<size_t>(charcount));
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide_length, &mb[0], charcount,
                      NULL, NULL);

  return mb;
}

}  // namespace

std::wstring StrErrorW(SystemErrorCode error) {
  constexpr DWORD kErrorMessageBufSize = 256;

  wchar_t msg_buf[kErrorMessageBufSize];

  // |msg_len| excluding the terminating null character.
  DWORD msg_len = ::FormatMessageW(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // dwFlags
      nullptr, // lpSource
      error,   // dwMessageId
      0,       // dwLanguageId
      msg_buf,
      kErrorMessageBufSize,
      nullptr);// Arguments
  if (msg_len == 0) {
    DWORD last_error = ::GetLastError();
    swprintf_s(msg_buf, kErrorMessageBufSize, 
               L"Error(%s) while retrieving error. (%s)",
               SystemErrorCodeToHexString<wchar_t>(last_error).c_str(),
               SystemErrorCodeToHexString<wchar_t>(error).c_str());
    return std::wstring(msg_buf);
  }

  // Messages returned by system end with line breaks. so remove it.
  while (msg_len != 0) {
    if (msg_buf[msg_len - 1] == '\r' || 
        msg_buf[msg_len - 1] == '\n') {
      msg_len--;
      continue;
    }
    break;
  }

  return msg_len == 0 ? std::wstring() : std::wstring(msg_buf, msg_len);
}

std::string StrError(SystemErrorCode error) {
  return WideToUTF8(StrErrorW(error));
}

}  // nmaespace logging
}  // namespace cr


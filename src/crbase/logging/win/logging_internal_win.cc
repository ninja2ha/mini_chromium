// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/logging/win/logging_internal_win.h"

#include "crbase/win/windows_types.h"

namespace {

std::wstring SystemErrorCode2Hex(cr::logging::SystemErrorCode error_code) {
  static_assert(sizeof(error_code) == 4, "Invalid type of SystemErrorCode!");
  wchar_t str[] = L"0x00000000";
  for (int i = 7; i >= 0; i--) {
	cr::logging::SystemErrorCode chr = error_code & 0xf;
    str[i + 2] 
		= static_cast<char>(chr >= 10 ? (chr - 10) + L'A' : (chr + L'0'));
    error_code >>= 4;
  }
  return str;
}

// Copied from chromium/base/strings/sys_string_conversions_win.cc
std::string SysWideToUTF8(const std::wstring& wide) {
  int wide_length = static_cast<int>(wide.length());
  if (wide_length == 0) {
    return std::string();
  }

  // Compute the length of the buffer we'll need.
  int charcount = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide_length,
                                      NULL, 0, NULL, NULL);
  if (charcount == 0) {
    return std::string();
  }

  std::string mb;
  mb.resize(static_cast<size_t>(charcount));
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide_length, &mb[0], charcount,
                      NULL, NULL);

  return mb;
}

}  // namespace

namespace cr {
namespace logging {
namespace internal {

std::string strerror(SystemErrorCode error_code) {
  constexpr DWORD kErrorMessageBufferSize = 256;
  wchar_t msgbuf[kErrorMessageBufferSize];
  DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD len = ::FormatMessageW(flags, nullptr, error_code, 0, msgbuf,
                               kErrorMessageBufferSize, nullptr);
  DWORD format_err = GetLastError();
  if (len) {
    // Messages returned by system end with line breaks. so remove it.
    while (--len) {
      if (msgbuf[len] == '\r' || msgbuf[len] == '\n') {
        msgbuf[len] = '\0';
        continue;
      }
      break;
    }
	std::wstring wide = msgbuf;
	wide += SystemErrorCode2Hex(error_code);
    return SysWideToUTF8(wide);
  }

  std::wstring wide;
  wide = L"Error (";
  wide += SystemErrorCode2Hex(format_err);
  wide += L") while retrieving error. (";
  wide += SystemErrorCode2Hex(error_code);
  wide += L")";
  return SysWideToUTF8(wide);
}

}  // namespace internal
}  // namespace loggging
}  // namespace cr
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/win/com/scoped_hstring.h"

#include <winstring.h>

#include "crbase/numerics/safe_conversions.h"
#include "crbase/process/memory.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/utf_string_conversions.h"

namespace cr {

namespace {

typedef HRESULT (WINAPI* WindowsCreateStringPtr)(
    PCNZWCH, UINT32, HSTRING*);
typedef HRESULT (WINAPI* WindowsDeleteStringPtr)(HSTRING);
typedef PCWSTR (WINAPI* WindowsGetStringRawBufferPtr)(HSTRING, UINT32*);

static bool g_load_succeeded = false;

FARPROC LoadComBaseFunction(const char* function_name) {
  static HMODULE const handle =
      ::LoadLibraryExW(L"combase.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
  return handle ? ::GetProcAddress(handle, function_name) : nullptr;
}

WindowsCreateStringPtr GetWindowsCreateString() {
  static WindowsCreateStringPtr const function =
      reinterpret_cast<WindowsCreateStringPtr>(
          LoadComBaseFunction("WindowsCreateString"));
  return function;
}

WindowsDeleteStringPtr GetWindowsDeleteString() {
  static WindowsDeleteStringPtr const function =
      reinterpret_cast<WindowsDeleteStringPtr>(
          LoadComBaseFunction("WindowsDeleteString"));
  return function;
}

WindowsGetStringRawBufferPtr GetWindowsGetStringRawBuffer() {
  static WindowsGetStringRawBufferPtr const function =
      reinterpret_cast<WindowsGetStringRawBufferPtr>(
          LoadComBaseFunction("WindowsGetStringRawBuffer"));
  return function;
}

HRESULT WindowsCreateString(const wchar_t* src,
                            uint32_t len,
                            HSTRING* out_hstr) {
  WindowsCreateStringPtr create_string_func =
      GetWindowsCreateString();
  if (!create_string_func)
    return E_FAIL;
  return create_string_func(src, len, out_hstr);
}

HRESULT WindowsDeleteString(HSTRING hstr) {
  WindowsDeleteStringPtr delete_string_func =
      GetWindowsDeleteString();
  if (!delete_string_func)
    return E_FAIL;
  return delete_string_func(hstr);
}

const wchar_t* WindowsGetStringRawBuffer(HSTRING hstr, uint32_t* out_len) {
  WindowsGetStringRawBufferPtr get_string_raw_buffer_func =
      GetWindowsGetStringRawBuffer();
  if (!get_string_raw_buffer_func) {
    *out_len = 0;
    return nullptr;
  }
  return get_string_raw_buffer_func(hstr, out_len);
}

}  // namespace

namespace internal {

// static
void ScopedHStringTraits::Free(HSTRING hstr) {
  cr::WindowsDeleteString(hstr);
}

}  // namespace internal

namespace win {

ScopedHString::ScopedHString(HSTRING hstr) : ScopedGeneric(hstr) {
  CR_DCHECK(g_load_succeeded);
}

// static
ScopedHString ScopedHString::Create(StringPiece16 str) {
  CR_DCHECK(g_load_succeeded);
  HSTRING hstr;
  HRESULT hr = cr::WindowsCreateString(
      str.data(), checked_cast<UINT32>(str.length()), &hstr);
  if (SUCCEEDED(hr))
    return ScopedHString(hstr);
  if (hr == E_OUTOFMEMORY) {
    // This size is an approximation. The actual size likely includes
    // sizeof(HSTRING_HEADER) as well.
    cr::TerminateBecauseOutOfMemory((str.length() + 1) * sizeof(wchar_t));
  }
  CR_DLOG(Error) << "Failed to create HSTRING" << std::hex << hr;
  return ScopedHString(nullptr);
}

// static
ScopedHString ScopedHString::Create(StringPiece str) {
  return Create(UTF8ToWide(str));
}

// static
bool ScopedHString::ResolveCoreWinRTStringDelayload() {
  // TODO(finnur): Add AssertIOAllowed once crbug.com/770193 is fixed.

  static const bool load_succeeded = []() {
    bool success = GetWindowsCreateString() && GetWindowsDeleteString() &&
                   GetWindowsGetStringRawBuffer();
    g_load_succeeded = success;
    return success;
  }();
  return load_succeeded;
}

StringPiece16 ScopedHString::Get() const {
  UINT32 length = 0;
  const wchar_t* buffer = cr::WindowsGetStringRawBuffer(get(), &length);
  return StringPiece16(buffer, length);
}

std::string ScopedHString::GetAsUTF8() const {
  std::string utf8;
  cr::StringPiece16 wide = Get();
  cr::WideToUTF8(wide.data(), wide.length(), &utf8);
  return utf8;
}

}  // namespace win
}  // namespace cr

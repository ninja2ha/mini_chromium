// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/files/win/file_version_info_win.h"

#include <stddef.h>

#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/files/file_path.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/stl_util.h"
#include "crbase/strings/string_util.h"
///#include "base/threading/scoped_blocking_call.h"
#include "crbase/win/resource_util.h"

namespace cr{

namespace {

struct LanguageAndCodePage {
  WORD language;
  WORD code_page;
};

// Returns the \VarFileInfo\Translation value extracted from the
// VS_VERSION_INFO resource in |data|.
LanguageAndCodePage* GetTranslate(const void* data) {
  static constexpr wchar_t kTranslation[] = L"\\VarFileInfo\\Translation";
  LPVOID translate = nullptr;
  UINT dummy_size;
  if (::VerQueryValueW(data, kTranslation, &translate, &dummy_size))
    return static_cast<LanguageAndCodePage*>(translate);
  return nullptr;
}

const VS_FIXEDFILEINFO& GetVsFixedFileInfo(const void* data) {
  static constexpr wchar_t kRoot[] = L"\\";
  LPVOID fixed_file_info = nullptr;
  UINT dummy_size;
  CR_CHECK(::VerQueryValueW(data, kRoot, &fixed_file_info, &dummy_size));
  return *static_cast<VS_FIXEDFILEINFO*>(fixed_file_info);
}

}  // namespace

FileVersionInfoWin::~FileVersionInfoWin() = default;

// static
std::unique_ptr<FileVersionInfo>
FileVersionInfo::CreateFileVersionInfoForModule(HMODULE module) {
  void* data;
  size_t version_info_length;
  const bool has_version_resource = cr::win::GetResourceFromModule(
      module, VS_VERSION_INFO, reinterpret_cast<LPCWSTR>(RT_VERSION), 
      &data, &version_info_length);
  if (!has_version_resource)
    return nullptr;

  const LanguageAndCodePage* translate = GetTranslate(data);
  if (!translate)
    return nullptr;

  return cr::WrapUnique(
      new FileVersionInfoWin(data, translate->language, translate->code_page));
}

// static
std::unique_ptr<FileVersionInfo> FileVersionInfo::CreateFileVersionInfo(
    const cr::FilePath& file_path) {
  return FileVersionInfoWin::CreateFileVersionInfoWin(file_path);
}

// static
std::unique_ptr<FileVersionInfoWin>
FileVersionInfoWin::CreateFileVersionInfoWin(const cr::FilePath& file_path) {
  ///cr::ScopedBlockingCall scoped_blocking_call(FROM_HERE,
  ///                                              cr::BlockingType::MAY_BLOCK);

  DWORD dummy;
  const wchar_t* path = file_path.value().c_str();
  const DWORD length = ::GetFileVersionInfoSizeW(path, &dummy);
  if (length == 0)
    return nullptr;

  std::vector<uint8_t> data(length, 0);

  if (!::GetFileVersionInfoW(path, dummy, length, data.data()))
    return nullptr;

  const LanguageAndCodePage* translate = GetTranslate(data.data());
  if (!translate)
    return nullptr;

  return cr::WrapUnique(new FileVersionInfoWin(
      std::move(data), translate->language, translate->code_page));
}

std::u16string FileVersionInfoWin::company_name() {
  return GetStringValue(u"CompanyName");
}

std::u16string FileVersionInfoWin::company_short_name() {
  return GetStringValue(u"CompanyShortName");
}

std::u16string FileVersionInfoWin::internal_name() {
  return GetStringValue(u"InternalName");
}

std::u16string FileVersionInfoWin::product_name() {
  return GetStringValue(u"ProductName");
}

std::u16string FileVersionInfoWin::product_short_name() {
  return GetStringValue(u"ProductShortName");
}

std::u16string FileVersionInfoWin::product_version() {
  return GetStringValue(u"ProductVersion");
}

std::u16string FileVersionInfoWin::file_description() {
  return GetStringValue(u"FileDescription");
}

std::u16string FileVersionInfoWin::file_version() {
  return GetStringValue(u"FileVersion");
}

std::u16string FileVersionInfoWin::original_filename() {
  return GetStringValue(u"OriginalFilename");
}

std::u16string FileVersionInfoWin::special_build() {
  return GetStringValue(u"SpecialBuild");
}

bool FileVersionInfoWin::GetValue(const char16_t* name,
                                  std::u16string* value) const {
  const struct LanguageAndCodePage lang_codepages[] = {
      // Use the language and codepage from the DLL.
      {language_, code_page_},
      // Use the default language and codepage from the DLL.
      {::GetUserDefaultLangID(), code_page_},
      // Use the language from the DLL and Latin codepage (most common).
      {language_, 1252},
      // Use the default language and Latin codepage (most common).
      {::GetUserDefaultLangID(), 1252},
  };

  for (const auto& lang_codepage : lang_codepages) {
    wchar_t sub_block[MAX_PATH];
    _snwprintf_s(sub_block, MAX_PATH, MAX_PATH,
                 L"\\StringFileInfo\\%04x%04x\\%ls", lang_codepage.language,
                 lang_codepage.code_page, cr::as_wcstr(name));
    LPVOID value_ptr = nullptr;
    uint32_t size;
    BOOL r = ::VerQueryValueW(data_, sub_block, &value_ptr, &size);
    if (r && value_ptr && size) {
      value->assign(static_cast<char16_t*>(value_ptr), size - 1);
      return true;
    }
  }
  return false;
}

std::u16string FileVersionInfoWin::GetStringValue(const char16_t* name) const {
  std::u16string str;
  GetValue(name, &str);
  return str;
}

cr::Version FileVersionInfoWin::GetFileVersion() const {
  return cr::Version({HIWORD(fixed_file_info_.dwFileVersionMS),
                      LOWORD(fixed_file_info_.dwFileVersionMS),
                      HIWORD(fixed_file_info_.dwFileVersionLS),
                      LOWORD(fixed_file_info_.dwFileVersionLS)});
}

FileVersionInfoWin::FileVersionInfoWin(std::vector<uint8_t>&& data,
                                       WORD language,
                                       WORD code_page)
    : owned_data_(std::move(data)),
      data_(owned_data_.data()),
      language_(language),
      code_page_(code_page),
      fixed_file_info_(GetVsFixedFileInfo(data_)) {
  CR_DCHECK(!owned_data_.empty());
}

FileVersionInfoWin::FileVersionInfoWin(void* data,
                                       WORD language,
                                       WORD code_page)
    : data_(data),
      language_(language),
      code_page_(code_page),
      fixed_file_info_(GetVsFixedFileInfo(data)) {
  CR_DCHECK(data_);
}

}  // namespace

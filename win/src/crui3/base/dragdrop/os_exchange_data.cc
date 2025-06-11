// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/dragdrop/os_exchange_data.h"

#include <utility>
#include <vector>

#include "crbase/functional/callback.h"
#include "crbase/buffer/pickle.h"
#include "crui/base/clipboard/clipboard_format_type.h"
#include "crui/base/dragdrop/os_exchange_data_provider_factory.h"
#include "crui/base/build_platform.h"

#include "crurl/gurl.h"

namespace crui {

OSExchangeData::DownloadFileInfo::DownloadFileInfo(
    const cr::FilePath& filename,
    std::unique_ptr<DownloadFileProvider> downloader)
    : filename(filename), downloader(std::move(downloader)) {}

OSExchangeData::DownloadFileInfo::~DownloadFileInfo() = default;

OSExchangeData::OSExchangeData()
    : provider_(OSExchangeDataProviderFactory::CreateProvider()) {
}

OSExchangeData::OSExchangeData(std::unique_ptr<Provider> provider)
    : provider_(std::move(provider)) {
}

OSExchangeData::~OSExchangeData() {
}

void OSExchangeData::MarkOriginatedFromRenderer() {
  provider_->MarkOriginatedFromRenderer();
}

bool OSExchangeData::DidOriginateFromRenderer() const {
  return provider_->DidOriginateFromRenderer();
}

void OSExchangeData::SetString(const cr::string16& data) {
  provider_->SetString(data);
}

void OSExchangeData::SetURL(const crurl::GURL& url, 
                            const cr::string16& title) {
  provider_->SetURL(url, title);
}

void OSExchangeData::SetFilename(const cr::FilePath& path) {
  provider_->SetFilename(path);
}

void OSExchangeData::SetFilenames(
    const std::vector<FileInfo>& filenames) {
  provider_->SetFilenames(filenames);
}

void OSExchangeData::SetPickledData(const ClipboardFormatType& format,
                                    const cr::Pickle& data) {
  provider_->SetPickledData(format, data);
}

bool OSExchangeData::GetString(cr::string16* data) const {
  return provider_->GetString(data);
}

bool OSExchangeData::GetURLAndTitle(FilenameToURLPolicy policy,
                                    crurl::GURL* url,
                                    cr::string16* title) const {
  return provider_->GetURLAndTitle(policy, url, title);
}

bool OSExchangeData::GetFilename(cr::FilePath* path) const {
  return provider_->GetFilename(path);
}

bool OSExchangeData::GetFilenames(std::vector<FileInfo>* filenames) const {
  return provider_->GetFilenames(filenames);
}

bool OSExchangeData::GetPickledData(const ClipboardFormatType& format,
                                    cr::Pickle* data) const {
  return provider_->GetPickledData(format, data);
}

bool OSExchangeData::HasString() const {
  return provider_->HasString();
}

bool OSExchangeData::HasURL(FilenameToURLPolicy policy) const {
  return provider_->HasURL(policy);
}

bool OSExchangeData::HasFile() const {
  return provider_->HasFile();
}

bool OSExchangeData::HasCustomFormat(const ClipboardFormatType& format) const {
  return provider_->HasCustomFormat(format);
}

bool OSExchangeData::HasAnyFormat(
    int formats,
    const std::set<ClipboardFormatType>& format_types) const {
  if ((formats & STRING) != 0 && HasString())
    return true;
  if ((formats & URL) != 0 && HasURL(CONVERT_FILENAMES))
    return true;
#if defined(MINI_CHROMIUM_OS_WIN)
  if ((formats & FILE_CONTENTS) != 0 && provider_->HasFileContents())
    return true;
#endif
#if defined(MINI_CHROMIUM_USE_AURA)
  if ((formats & HTML) != 0 && provider_->HasHtml())
    return true;
#endif
  if ((formats & FILE_NAME) != 0 && provider_->HasFile())
    return true;
  for (const auto& format : format_types) {
    if (HasCustomFormat(format))
      return true;
  }
  return false;
}

#if defined(MINI_CHROMIUM_OS_WIN)
void OSExchangeData::SetFileContents(const cr::FilePath& filename,
                                     const std::string& file_contents) {
  provider_->SetFileContents(filename, file_contents);
}

bool OSExchangeData::GetFileContents(cr::FilePath* filename,
                                     std::string* file_contents) const {
  return provider_->GetFileContents(filename, file_contents);
}

bool OSExchangeData::HasVirtualFilenames() const {
  return provider_->HasVirtualFilenames();
}

bool OSExchangeData::GetVirtualFilenames(
    std::vector<FileInfo>* filenames) const {
  return provider_->GetVirtualFilenames(filenames);
}

bool OSExchangeData::GetVirtualFilesAsTempFiles(
    cr::OnceCallback<
        void(const std::vector<std::pair<cr::FilePath, cr::FilePath>>&)>
        callback) const {
  return provider_->GetVirtualFilesAsTempFiles(std::move(callback));
}

void OSExchangeData::SetDownloadFileInfo(DownloadFileInfo* download) {
  provider_->SetDownloadFileInfo(download);
}
#endif

#if defined(MINI_CHROMIUM_USE_AURA)
bool OSExchangeData::HasHtml() const {
  return provider_->HasHtml();
}

void OSExchangeData::SetHtml(const cr::string16& html, 
                             const crurl::GURL& base_url) {
  provider_->SetHtml(html, base_url);
}

bool OSExchangeData::GetHtml(cr::string16* html, 
                             crurl::GURL* base_url) const {
  return provider_->GetHtml(html, base_url);
}
#endif

}  // namespace crui

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_PE_FILE_VERSION_INFO_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_PE_FILE_VERSION_INFO_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "cr_base/base_export.h"
#include "cr_base/version.h"
#include "cr_base/win/windows_types.h"

struct tagVS_FIXEDFILEINFO;
typedef tagVS_FIXEDFILEINFO VS_FIXEDFILEINFO;

namespace cr {
class FilePath;

namespace win {

class CRBASE_EXPORT PeFileVersionInfo {
 public:
  PeFileVersionInfo(const PeFileVersionInfo&) = delete;
  PeFileVersionInfo& operator=(const PeFileVersionInfo&) = delete;
  ~PeFileVersionInfo();

  // Accessors to the different version properties.
  // Returns an empty string if the property is not found.
  std::u16string company_name();
  std::u16string company_short_name();
  std::u16string product_name();
  std::u16string product_short_name();
  std::u16string internal_name();
  std::u16string product_version();
  std::u16string special_build();
  std::u16string original_filename();
  std::u16string file_description();
  std::u16string file_version();

  // Lets you access other properties not covered above. |value| is only
  // modified if GetValue() returns true.
  bool GetValue(const char16_t* name, std::u16string* value) const;

  // Similar to GetValue but returns a std::u16string (empty string if the
  // property does not exist).
  std::u16string GetStringValue(const char16_t* name) const;

  // Get file version number in dotted version format.
  cr::Version GetFileVersion() const;
  
  static std::unique_ptr<PeFileVersionInfo> CreateFromModule(
      HMODULE module);
  static std::unique_ptr<PeFileVersionInfo> CreateFromFile(
      const cr::FilePath& file_path);

 private:
  // |data| is a VS_VERSION_INFO resource. |language| and |code_page| are
  // extracted from the \VarFileInfo\Translation value of |data|.
  PeFileVersionInfo(std::vector<uint8_t>&& data,
                    WORD language,
                    WORD code_page);
  PeFileVersionInfo(void* data, WORD language, WORD code_page);

  const std::vector<uint8_t> owned_data_;
  const void* const data_;
  const WORD language_;
  const WORD code_page_;

  // This is a reference for a portion of |data_|.
  const VS_FIXEDFILEINFO& fixed_file_info_;
};

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_PE_FILE_VERSION_INFO_H_

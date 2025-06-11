// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_DRAGDROP_FILE_INFO_FILE_INFO_H_
#define UI_BASE_DRAGDROP_FILE_INFO_FILE_INFO_H_

#include "crbase/files/file_path.h"
#include "crui/base/ui_export.h"

namespace crui {

// struct that bundles a file's path with an optional display name.
struct CRUI_EXPORT FileInfo {
  FileInfo();
  FileInfo(const cr::FilePath& path, const cr::FilePath& display_name);
  ~FileInfo();
  bool operator==(const FileInfo& other) const;

  cr::FilePath path;
  cr::FilePath display_name;  // Optional.
};

}  // namespace crui

#endif  // UI_BASE_DRAGDROP_FILE_INFO_FILE_INFO_H_

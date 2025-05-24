// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/dragdrop/file_info/file_info.h"

namespace crui {

FileInfo::FileInfo() = default;

FileInfo::FileInfo(const cr::FilePath& path,
                   const cr::FilePath& display_name)
    : path(path), display_name(display_name) {}

FileInfo::~FileInfo() = default;

bool FileInfo::operator==(const FileInfo& other) const {
  return path == other.path && display_name == other.display_name;
}

}  // namespace crui

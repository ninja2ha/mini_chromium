// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/process/process_iterator.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
typedef struct IUnknown IUnknown;
#include <windows.h>
#include <tlhelp32.h>

#include "cr_base/strings/string_util.h"

namespace cr {

ProcessIterator::ProcessIterator(const ProcessFilter* filter)
    : snapshot_(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)),
      filter_(filter) {}

ProcessIterator::~ProcessIterator() {
  CloseHandle(snapshot_);
}

bool ProcessIterator::CheckForNextProcess() {
  entry_.dwSize = sizeof(entry_);

  if (!started_iteration_) {
    started_iteration_ = true;
    return !!Process32FirstW(snapshot_, win::ToWinType(&entry_));
  }

  return !!Process32NextW(snapshot_, win::ToWinType(&entry_));
}

bool NamedProcessIterator::IncludeEntry() {
  FilePath::StringPieceType entry_exe_view(entry().exe_file());

  return FilePath::CompareEqualIgnoreCase(executable_name_, entry_exe_view) &&
         ProcessIterator::IncludeEntry();
}

}  // namespace cr
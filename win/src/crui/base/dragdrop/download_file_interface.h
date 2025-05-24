// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_DRAGDROP_DOWNLOAD_FILE_INTERFACE_H_
#define UI_BASE_DRAGDROP_DOWNLOAD_FILE_INTERFACE_H_


#include "crbase/memory/ref_counted.h"

#include "crui/base/ui_export.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <objidl.h>
#endif

namespace cr {
class FilePath;
}  // namespace cr

namespace crui {

// Defines the interface to observe the status of file download.
class CRUI_EXPORT DownloadFileObserver
    : public cr::RefCountedThreadSafe<DownloadFileObserver> {
 public:
  virtual void OnDownloadCompleted(const cr::FilePath& file_path) = 0;
  virtual void OnDownloadAborted() = 0;

 protected:
  friend class cr::RefCountedThreadSafe<DownloadFileObserver>;
  virtual ~DownloadFileObserver() = default;
};

// Defines the interface to control how a file is downloaded.
class CRUI_EXPORT DownloadFileProvider {
 public:
  virtual ~DownloadFileProvider() = default;

  // Starts the download asynchronously and returns immediately.
  virtual void Start(DownloadFileObserver* observer) = 0;

  // Returns true if the download succeeded and false otherwise. Waits until the
  // download is completed/cancelled/interrupted before returning.
  virtual bool Wait() = 0;

  // Cancels the download.
  virtual void Stop() = 0;
};

}  // namespace crui

#endif  // UI_BASE_DRAGDROP_DOWNLOAD_FILE_INTERFACE_H_

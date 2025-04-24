// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/files/file.h"

#include <utility>

#include "crbase/files/file_path.h"
///#include "base/files/file_tracing.h"
///#include "crbase/metrics/histogram.h"
#include "crbase/numerics/safe_conversions.h"
///#include "crbase/timer/elapsed_timer.h"
#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <errno.h>
#endif

namespace cr {

File::Info::Info() = default;

File::Info::~Info() = default;

File::File() = default;

File::File(const FilePath& path, uint32_t flags) : error_details_(FILE_OK) {
  Initialize(path, flags);
}

File::File(ScopedPlatformFile platform_file)
    : File(std::move(platform_file), false) {}

File::File(PlatformFile platform_file) : File(platform_file, false) {}

File::File(ScopedPlatformFile platform_file, bool async)
    : file_(std::move(platform_file)), error_details_(FILE_OK), async_(async) {
#if defined(MINI_CHROMIUM_OS_POSIX)
  CR_DCHECK(file_.get() >= -1);
#endif
}

File::File(PlatformFile platform_file, bool async)
    : file_(platform_file),
      error_details_(FILE_OK),
      async_(async) {
#if defined(MINI_CHROMIUM_OS_POSIX)
  CR_DCHECK(file_.get() >= -1);
#endif
}

File::File(Error error_details) : error_details_(error_details) {}

File::File(File&& other)
    : file_(other.TakePlatformFile()),
      ///tracing_path_(other.tracing_path_),
      error_details_(other.error_details()),
      created_(other.created()),
      async_(other.async_) {}

File::~File() {
  // Go through the AssertIOAllowed logic.
  Close();
}

File& File::operator=(File&& other) {
  Close();
  SetPlatformFile(other.TakePlatformFile());
  ///tracing_path_ = other.tracing_path_;
  error_details_ = other.error_details();
  created_ = other.created();
  async_ = other.async_;
  return *this;
}

void File::Initialize(const FilePath& path, uint32_t flags) {
  if (path.ReferencesParent()) {
#if defined(MINI_CHROMIUM_OS_WIN)
    ::SetLastError(ERROR_ACCESS_DENIED);
#elif defined(MINI_CHROMIUM_OS_POSIX)
    errno = EACCES;
#else
#error Unsupported platform
#endif
    error_details_ = FILE_ERROR_ACCESS_DENIED;
    return;
  }
  ///if (FileTracing::IsCategoryEnabled())
  ///  tracing_path_ = path;
  ///SCOPED_FILE_TRACE("Initialize");
  DoInitialize(path, flags);
}

bool File::ReadAndCheck(int64_t offset, Span<uint8_t> data) {
  int size = checked_cast<int>(data.size());
  return Read(offset, reinterpret_cast<char*>(data.data()), size) == size;
}

bool File::ReadAtCurrentPosAndCheck(Span<uint8_t> data) {
  int size = checked_cast<int>(data.size());
  return ReadAtCurrentPos(reinterpret_cast<char*>(data.data()), size) == size;
}

bool File::WriteAndCheck(int64_t offset, Span<const uint8_t> data) {
  int size = checked_cast<int>(data.size());
  return Write(offset, reinterpret_cast<const char*>(data.data()), size) ==
         size;
}

bool File::WriteAtCurrentPosAndCheck(Span<const uint8_t> data) {
  int size = checked_cast<int>(data.size());
  return WriteAtCurrentPos(reinterpret_cast<const char*>(data.data()), size) ==
         size;
}

// static
std::string File::ErrorToString(Error error) {
  switch (error) {
    case FILE_OK:
      return "FILE_OK";
    case FILE_ERROR_FAILED:
      return "FILE_ERROR_FAILED";
    case FILE_ERROR_IN_USE:
      return "FILE_ERROR_IN_USE";
    case FILE_ERROR_EXISTS:
      return "FILE_ERROR_EXISTS";
    case FILE_ERROR_NOT_FOUND:
      return "FILE_ERROR_NOT_FOUND";
    case FILE_ERROR_ACCESS_DENIED:
      return "FILE_ERROR_ACCESS_DENIED";
    case FILE_ERROR_TOO_MANY_OPENED:
      return "FILE_ERROR_TOO_MANY_OPENED";
    case FILE_ERROR_NO_MEMORY:
      return "FILE_ERROR_NO_MEMORY";
    case FILE_ERROR_NO_SPACE:
      return "FILE_ERROR_NO_SPACE";
    case FILE_ERROR_NOT_A_DIRECTORY:
      return "FILE_ERROR_NOT_A_DIRECTORY";
    case FILE_ERROR_INVALID_OPERATION:
      return "FILE_ERROR_INVALID_OPERATION";
    case FILE_ERROR_SECURITY:
      return "FILE_ERROR_SECURITY";
    case FILE_ERROR_ABORT:
      return "FILE_ERROR_ABORT";
    case FILE_ERROR_NOT_A_FILE:
      return "FILE_ERROR_NOT_A_FILE";
    case FILE_ERROR_NOT_EMPTY:
      return "FILE_ERROR_NOT_EMPTY";
    case FILE_ERROR_INVALID_URL:
      return "FILE_ERROR_INVALID_URL";
    case FILE_ERROR_IO:
      return "FILE_ERROR_IO";
    case FILE_ERROR_MAX:
      break;
  }

  CR_NOTREACHED();
  return "";
}

}  // namespace cr
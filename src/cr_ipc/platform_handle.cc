// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/platform_handle.h"

#include "crbase/logging/logging.h"
#include "crbase/stl_util.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/windows_types.h"
#include "crbase/win/scoped_handle.h"
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <unistd.h>

#include "crbase/files/scoped_file.h"
#endif

namespace mojo {

namespace {

#if defined(MINI_CHROMIUM_OS_WIN)
cr::win::ScopedHandle CloneHandle(const cr::win::ScopedHandle& handle) {
  CR_DCHECK(handle.IsValid());

  HANDLE dupe;
  BOOL result = ::DuplicateHandle(::GetCurrentProcess(), handle.Get(),
                                  ::GetCurrentProcess(), &dupe, 0, FALSE,
                                  DUPLICATE_SAME_ACCESS);
  if (!result)
    return cr::win::ScopedHandle();
  CR_DCHECK(dupe != INVALID_HANDLE_VALUE);
  return cr::win::ScopedHandle(dupe);
}
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
cr::ScopedFD CloneFD(const cr::ScopedFD& fd) {
  CR_DCHECK(fd.is_valid());
  return cr::ScopedFD(dup(fd.get()));
}
#endif

}  // namespace

PlatformHandle::PlatformHandle() = default;

PlatformHandle::PlatformHandle(PlatformHandle&& other) {
  *this = std::move(other);
}

#if defined(MINI_CHROMIUM_OS_WIN)
PlatformHandle::PlatformHandle(cr::win::ScopedHandle handle)
    : type_(Type::kHandle), handle_(std::move(handle)) {}
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
PlatformHandle::PlatformHandle(cr::ScopedFD fd)
    : type_(Type::kFd), fd_(std::move(fd)) {
}
#endif

PlatformHandle::~PlatformHandle() = default;

PlatformHandle& PlatformHandle::operator=(PlatformHandle&& other) {
  type_ = other.type_;
  other.type_ = Type::kNone;

#if defined(MINI_CHROMIUM_OS_WIN)
  handle_ = std::move(other.handle_);
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
  fd_ = std::move(other.fd_);
#endif

  return *this;
}

void PlatformHandle::reset() {
  type_ = Type::kNone;

#if defined(MINI_CHROMIUM_OS_WIN)
  handle_.Close();
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
  fd_.reset();
#endif
}

void PlatformHandle::release() {
  type_ = Type::kNone;

#if defined(MINI_CHROMIUM_OS_WIN)
  cr::ignore_result(handle_.Take());
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
  cr::ignore_result(fd_.release());
#endif
}

PlatformHandle PlatformHandle::Clone() const {
#if defined(MINI_CHROMIUM_OS_WIN)
  return PlatformHandle(CloneHandle(handle_));
#elif defined(MINI_CHROMIUM_OS_POSIX)
  return PlatformHandle(CloneFD(fd_));
#endif
}

}  // namespace mojo
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_PLATFORM_PLATFORM_HANDLE_H_
#define MOJO_PUBLIC_CPP_PLATFORM_PLATFORM_HANDLE_H_

#include "crbase/logging/logging.h"
#include "crbase/files/platform_file.h"
#include "cripc/ipc_export.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/scoped_handle.h"
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
#include "crbase/files/scoped_file.h"
#endif

namespace mojo {

// A PlatformHandle is a generic wrapper around a platform-specific system
// handle type, e.g. a POSIX file descriptor, Windows HANDLE, or macOS Mach
// port. This can wrap any of various such types depending on the host platform
// for which it's compiled.
//
// This is useful primarily for two reasons:
//
// - Interacting with the Mojo invitation API, which use OS primitives to
//   bootstrap Mojo IPC connections.
// - Interacting with Mojo platform handle wrapping and unwrapping API, which
//   allows handles to OS primitives to be transmitted over Mojo IPC with a
//   stable wire representation via Mojo handles.
//
// NOTE: This assumes ownership if the handle it represents.
class CRIPC_EXPORT PlatformHandle {
 public:
  enum class Type {
    kNone,
#if defined(MINI_CHROMIUM_OS_WIN)
    kHandle,
#elif defined(MINI_CHROMIUM_OS_POSIX)
    kFd,
#endif
  };

  PlatformHandle(const PlatformHandle&) = delete;
  PlatformHandle& operator=(const PlatformHandle&) = delete;

  PlatformHandle();
  PlatformHandle(PlatformHandle&& other);

#if defined(MINI_CHROMIUM_OS_WIN)
  explicit PlatformHandle(cr::win::ScopedHandle handle);
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
  explicit PlatformHandle(cr::ScopedFD fd);
#endif

  ~PlatformHandle();

  PlatformHandle& operator=(PlatformHandle&& other);

  Type type() const { return type_; }

  void reset();

  // Relinquishes ownership of the underlying handle, regardless of type, and
  // discards its value. To release and obtain the underlying handle value, use
  // one of the specific |Release*()| methods below.
  void release();

  // Duplicates the underlying platform handle, returning a new PlatformHandle
  // which owns it.
  PlatformHandle Clone() const;

#if defined(MINI_CHROMIUM_OS_WIN)
  bool is_valid() const { return is_valid_handle(); }
  bool is_valid_handle() const { return handle_.IsValid(); }
  bool is_handle() const { return type_ == Type::kHandle; }
  const cr::win::ScopedHandle& GetHandle() const { return handle_; }
  cr::win::ScopedHandle TakeHandle() {
    CR_DCHECK(type_ == Type::kHandle);
    type_ = Type::kNone;
    return std::move(handle_);
  }
  HANDLE ReleaseHandle() CR_WARN_UNUSED_RESULT {
    CR_DCHECK(type_ == Type::kHandle);
    type_ = Type::kNone;
    return handle_.Take();
  }
#elif defined(MINI_CHROMIUM_OS_POSIX)
  bool is_valid() const { return is_valid_fd(); }
#else
#error "Unsupported platform."
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
  bool is_valid_fd() const { return fd_.is_valid(); }
  bool is_fd() const { return type_ == Type::kFd; }
  const cr::ScopedFD& GetFD() const { return fd_; }
  cr::ScopedFD TakeFD() {
    if (type_ == Type::kFd)
      type_ = Type::kNone;
    return std::move(fd_);
  }
  int ReleaseFD() CR_WARN_UNUSED_RESULT {
    if (type_ == Type::kFd)
      type_ = Type::kNone;
    return fd_.release();
  }
#endif

  bool is_valid_platform_file() const {
#if defined(MINI_CHROMIUM_OS_POSIX)
    return is_valid_fd();
#elif defined(MINI_CHROMIUM_OS_WIN)
    return is_valid_handle();
#else
#error "Unsupported platform"
#endif
  }
  cr::ScopedPlatformFile TakePlatformFile() {
#if defined(MINI_CHROMIUM_OS_POSIX)
    return TakeFD();
#elif defined(MINI_CHROMIUM_OS_WIN)
    return TakeHandle();
#else
#error "Unsupported platform"
#endif
  }
  cr::PlatformFile ReleasePlatformFile() CR_WARN_UNUSED_RESULT {
#if defined(MINI_CHROMIUM_OS_POSIX)
    return ReleaseFD();
#elif defined(MINI_CHROMIUM_OS_WIN)
    return ReleaseHandle();
#else
#error "Unsupported platform"
#endif
  }

 private:
  Type type_ = Type::kNone;

#if defined(MINI_CHROMIUM_OS_WIN)
  cr::win::ScopedHandle handle_;
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
  cr::ScopedFD fd_;
#endif
};

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_PLATFORM_PLATFORM_HANDLE_H_
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_REF_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_REF_H_

#include "cr_base/compiler_config.h"

#include "cr_base/base_export.h"

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <pthread.h>
#include <unistd.h>
#elif defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"
#endif

namespace cr {

// Used for thread checking and debugging.
// Meant to be as fast as possible.
// These are produced by PlatformThreadRef::Current(), and used to later
// check if we are on the same thread or not by using ==. These are safe
// to copy between threads, but can't be copied to another process as they
// have no meaning there. Also, the internal identifier can be re-used
// after a thread dies, so a PlatformThreadRef cannot be reliably used
// to distinguish a new thread from an old, dead thread.
class CRBASE_EXPORT PlatformThreadRef {
 public:
#if defined(MINI_CHROMIUM_OS_WIN)
  typedef DWORD RefType;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  typedef pthread_t RefType;
#else
#error Unsupported platform
#endif

  constexpr PlatformThreadRef() : id_(0) {}

  explicit constexpr PlatformThreadRef(RefType id) : id_(id) {}

  bool operator==(PlatformThreadRef other) const {
    return id_ == other.id_;
  }

  bool operator!=(PlatformThreadRef other) const { return id_ != other.id_; }

  bool is_null() const {
    return id_ == 0;
  }

  // Gets the current thread reference, which can be used to check if
  // we're on the right thread quickly.
  static PlatformThreadRef Current();

 private:
  RefType id_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_REF_H_

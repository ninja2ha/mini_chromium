// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/files/scoped_file.h"

#include "crbase/logging/logging.h"
#include "cr/build_config.h"

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <errno.h>
#include <unistd.h>

#include "crbase/posix/eintr_wrapper.h"
#endif

namespace cr {
namespace internal {

#if defined(MINI_CHROMIUM_OS_POSIX)

// static
void ScopedFDCloseTraits::Free(int fd) {
  // It's important to crash here.
  // There are security implications to not closing a file descriptor
  // properly. As file descriptors are "capabilities", keeping them open
  // would make the current process keep access to a resource. Much of
  // Chrome relies on being able to "drop" such access.
  // It's especially problematic on Linux with the setuid sandbox, where
  // a single open directory would bypass the entire security model.
  int ret = IGNORE_EINTR(close(fd));

#if defined(MINI_CHROMIUM_OS_LINUX)
  // NB: Some file descriptors can return errors from close() e.g. network
  // filesystems such as NFS and Linux input devices. On Linux, macOS, and
  // Fuchsia's POSIX layer, errors from close other than EBADF do not indicate
  // failure to actually close the fd.
  if (ret != 0 && errno != EBADF)
    ret = 0;
#endif

  CR_PCHECK(0 == ret);
}

#endif  // MINI_CHROMIUM_OS_POSIX

}  // namespace internal
}  // namespace cr

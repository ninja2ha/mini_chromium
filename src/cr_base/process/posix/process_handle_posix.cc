// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/process/process_handle.h"

#include <unistd.h>

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_OS_OPENBSD)
#include <stddef.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#elif defined(MINI_CHROMIUM_OS_FREEBSD)
#include <limits.h>
#include <stddef.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>
#elif defined(MINI_CHROMIUM_OS_LINUX)
#include "cr_base/files/file_util.h"
#endif

#include "cr_base/stl_util.h"

namespace cr {

ProcessId GetCurrentProcId() {
  return getpid();
}

ProcessHandle GetCurrentProcessHandle() {
  return GetCurrentProcId();
}

ProcessId GetProcId(ProcessHandle process) {
  return process;
}

#if defined(MINI_CHROMIUM_OS_OPENBSD)

ProcessId GetParentProcessId(ProcessHandle process) {
  struct kinfo_proc info;
  size_t length;
  int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, process,
                sizeof(struct kinfo_proc), 0 };

  if (sysctl(mib, cr::size(mib), NULL, &length, NULL, 0) < 0)
    return -1;

  mib[5] = (length / sizeof(struct kinfo_proc));

  if (sysctl(mib, cr::size(mib), &info, &length, NULL, 0) < 0)
    return -1;

  return info.p_ppid;
}

FilePath GetProcessExecutablePath(ProcessHandle process) {
  struct kinfo_proc kp;
  size_t len;
  int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, process,
                sizeof(struct kinfo_proc), 0 };

  if (sysctl(mib, cr::size(mib), NULL, &len, NULL, 0) == -1)
    return FilePath();
  mib[5] = (len / sizeof(struct kinfo_proc));
  if (sysctl(mib, cr::size(mib), &kp, &len, NULL, 0) < 0)
    return FilePath();

  if ((kp.p_flag & P_SYSTEM) != 0)
    return FilePath();

  if (strcmp(kp.p_comm, "chrome") == 0)
    return FilePath(kp.p_comm);

  return FilePath();
}

#elif defined(MINI_CHROMIUM_OS_FREEBSD)

ProcessId GetParentProcessId(ProcessHandle process) {
  struct kinfo_proc info;
  size_t length;
  int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, process };

  if (sysctl(mib, cr::size(mib), &info, &length, NULL, 0) < 0)
    return -1;

  return info.ki_ppid;
}

FilePath GetProcessExecutablePath(ProcessHandle process) {
  char pathname[PATH_MAX];
  size_t length;
  int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, process };

  length = sizeof(pathname);

  if (sysctl(mib, cr::size(mib), pathname, &length, NULL, 0) < 0 ||
      length == 0) {
    return FilePath();
  }

  return FilePath(std::string(pathname));
}
#endif

}  // namespace cr
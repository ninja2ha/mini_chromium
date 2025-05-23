// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_HANDLE_H_
#define MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_HANDLE_H_

#include <stdint.h>
#include <sys/types.h>

#include "crbase/base_export.h"
#include "crbase/files/file_path.h"
#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#endif

namespace cr {

// ProcessHandle is a platform specific type which represents the underlying OS
// handle to a process.
// ProcessId is a number which identifies the process in the OS.
#if defined(MINI_CHROMIUM_OS_WIN)
typedef HANDLE ProcessHandle;
typedef DWORD ProcessId;
typedef HANDLE UserTokenHandle;
static constexpr ProcessHandle kNullProcessHandle = NULL;
static constexpr ProcessId kNullProcessId = 0;
#elif defined(MINI_CHROMIUM_OS_POSIX)
// On POSIX, our ProcessHandle will just be the PID.
typedef pid_t ProcessHandle;
typedef pid_t ProcessId;
const ProcessHandle kNullProcessHandle = 0;
const ProcessId kNullProcessId = 0;
#endif  // defined(MINI_CHROMIUM_OS_WIN)

// To print ProcessIds portably use CrPRIdPid (based on PRIuS and friends from
// C99 and format_macros.h) like this:
// base::StringPrintf("PID is %" CrPRIdPid ".\n", pid);
#if defined(MINI_CHROMIUM_OS_WIN)
#define CrPRIdPid "ld"
#else
#define CrPRIdPid "d"
#endif

class UniqueProcId {
 public:
  explicit UniqueProcId(ProcessId value) : value_(value) {}
  UniqueProcId(const UniqueProcId& other) = default;
  UniqueProcId& operator=(const UniqueProcId& other) = default;

  // Returns the process PID. WARNING: On some platforms, the pid may not be
  // valid within the current process sandbox.
  ProcessId GetUnsafeValue() const { return value_; }

  bool operator==(const UniqueProcId& other) const {
    return value_ == other.value_;
  }

  bool operator!=(const UniqueProcId& other) const {
    return value_ != other.value_;
  }

  bool operator<(const UniqueProcId& other) const {
    return value_ < other.value_;
  }

  bool operator<=(const UniqueProcId& other) const {
    return value_ <= other.value_;
  }

  bool operator>(const UniqueProcId& other) const {
    return value_ > other.value_;
  }

  bool operator>=(const UniqueProcId& other) const {
    return value_ >= other.value_;
  }

 private:
  ProcessId value_;
};

std::ostream& operator<<(std::ostream& os, const UniqueProcId& obj);

// Returns the id of the current process.
// Note that on some platforms, this is not guaranteed to be unique across
// processes (use GetUniqueIdForProcess if uniqueness is required).
CRBASE_EXPORT ProcessId GetCurrentProcId();

// Returns a unique ID for the current process. The ID will be unique across all
// currently running processes within the chrome session, but IDs of terminated
// processes may be reused.
CRBASE_EXPORT UniqueProcId GetUniqueIdForProcess();

#if defined(MINI_CHROMIUM_OS_LINUX)
// When a process is started in a different PID namespace from the browser
// process, this function must be called with the process's PID in the browser's
// PID namespace in order to initialize its unique ID. Not thread safe.
// WARNING: To avoid inconsistent results from GetUniqueIdForProcess, this
// should only be called very early after process startup - ideally as soon
// after process creation as possible.
CRBASE_EXPORT void InitUniqueIdForProcessInPidNamespace(
    ProcessId pid_outside_of_namespace);
#endif

// Returns the ProcessHandle of the current process.
CRBASE_EXPORT ProcessHandle GetCurrentProcessHandle();

// Returns the process ID for the specified process. This is functionally the
// same as Windows' GetProcessId(), but works on versions of Windows before Win
// XP SP1 as well.
// DEPRECATED. New code should be using Process::Pid() instead.
// Note that on some platforms, this is not guaranteed to be unique across
// processes.
CRBASE_EXPORT ProcessId GetProcId(ProcessHandle process);

// Returns the ID for the parent of the given process. Not available on Fuchsia.
// Returning a negative value indicates an error, such as if the |process| does
// not exist. Returns 0 when |process| has no parent process.
CRBASE_EXPORT ProcessId GetParentProcessId(ProcessHandle process);

#if defined(MINI_CHROMIUM_OS_POSIX)
// Returns the path to the executable of the given process.
CRBASE_EXPORT FilePath GetProcessExecutablePath(ProcessHandle process);
#endif

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_HANDLE_H_
// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_H_
#define MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_H_

#include "crbase/base_export.h"
#include "crbase/process/process_handle.h"
#include "crbase/time/time.h"
#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/scoped_handle.h"
#endif

namespace cr {

// Provides a move-only encapsulation of a process.
//
// This object is not tied to the lifetime of the underlying process: the
// process may be killed and this object may still around, and it will still
// claim to be valid. The actual behavior in that case is OS dependent like so:
//
// Windows: The underlying ProcessHandle will be valid after the process dies
// and can be used to gather some information about that process, but most
// methods will obviously fail.
//
// POSIX: The underlying ProcessHandle is not guaranteed to remain valid after
// the process dies, and it may be reused by the system, which means that it may
// end up pointing to the wrong process.
class CRBASE_EXPORT Process {
 public:
  Process(const Process&) = delete;
  Process& operator=(const Process&) = delete;

  // On Windows, this takes ownership of |handle|. On POSIX, this does not take
  // ownership of |handle|.
  explicit Process(ProcessHandle handle = kNullProcessHandle);

  Process(Process&& other);

  // The destructor does not terminate the process.
  ~Process();

  Process& operator=(Process&& other);

  // Returns an object for the current process.
  static Process Current();

  // Returns a Process for the given |pid|.
  static Process Open(ProcessId pid);

  // Returns a Process for the given |pid|. On Windows the handle is opened
  // with more access rights and must only be used by trusted code (can read the
  // address space and duplicate handles).
  static Process OpenWithExtraPrivileges(ProcessId pid);

#if defined(MINI_CHROMIUM_OS_WIN)
  // Returns a Process for the given |pid|, using some |desired_access|.
  // See ::OpenProcess documentation for valid |desired_access|.
  static Process OpenWithAccess(ProcessId pid, DWORD desired_access);
#endif

  // Creates an object from a |handle| owned by someone else.
  // Don't use this for new code. It is only intended to ease the migration to
  // a strict ownership model.
  // TODO(rvargas) crbug.com/417532: Remove this code.
  static Process DeprecatedGetProcessFromHandle(ProcessHandle handle);

  // Returns true if processes can be backgrounded.
  static bool CanBackgroundProcesses();

  // Terminates the current process immediately with |exit_code|.
  static void TerminateCurrentProcessImmediately(int exit_code);

  // Returns true if this objects represents a valid process.
  bool IsValid() const;

  // Returns a handle for this process. There is no guarantee about when that
  // handle becomes invalid because this object retains ownership.
  ProcessHandle Handle() const;

  // Returns a second object that represents this process.
  Process Duplicate() const;

  // Get the PID for this process.
  ProcessId Pid() const;

  // Get the creation time for this process. Since the Pid can be reused after a
  // process dies, it is useful to use both the Pid and the creation time to
  // uniquely identify a process.
  //
  // Not available on Android because /proc/stat/ cannot be accessed on O+.
  // https://issuetracker.google.com/issues/37140047
  Time CreationTime() const;

  // Returns true if this process is the current process.
  bool is_current() const;

  // Close the process handle. This will not terminate the process.
  void Close();

  // Returns true if this process is still running. This is only safe on Windows
  // (and maybe Fuchsia?), because the ProcessHandle will keep the zombie
  // process information available until itself has been released. But on Posix,
  // the OS may reuse the ProcessId.
#if defined(MINI_CHROMIUM_OS_WIN)
  bool IsRunning() const {
    return !WaitForExitWithTimeout(cr::TimeDelta(), nullptr);
  }
#endif

  // Terminates the process with extreme prejudice. The given |exit_code| will
  // be the exit code of the process. If |wait| is true, this method will wait
  // for up to one minute for the process to actually terminate.
  // Returns true if the process terminates within the allowed time.
  // NOTE: On POSIX |exit_code| is ignored.
  bool Terminate(int exit_code, bool wait) const;

  // Waits for the process to exit. Returns true on success.
  // On POSIX, if the process has been signaled then |exit_code| is set to -1.
  // On Linux this must be a child process, however on Mac and Windows it can be
  // any process.
  // NOTE: |exit_code| is optional, nullptr can be passed if the exit code is
  // not required.
  bool WaitForExit(int* exit_code) const;

  // Same as WaitForExit() but only waits for up to |timeout|.
  // NOTE: |exit_code| is optional, nullptr can be passed if the exit code
  // is not required.
  bool WaitForExitWithTimeout(TimeDelta timeout, int* exit_code) const;

  // A process is backgrounded when it's priority is lower than normal.
  // Return true if this process is backgrounded, false otherwise.
  bool IsProcessBackgrounded() const;

  // Set a process as backgrounded. If value is true, the priority of the
  // process will be lowered. If value is false, the priority of the process
  // will be made "normal" - equivalent to default process priority.
  // Returns true if the priority was changed, false otherwise.
  bool SetProcessBackgrounded(bool value);

  // Returns an integer representing the priority of a process. The meaning
  // of this value is OS dependent.
  int GetPriority() const;

 private:
#if defined(MINI_CHROMIUM_OS_WIN)
  win::ScopedHandle process_;
#else
  ProcessHandle process_;
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
  bool is_current_process_;
#endif
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_H_
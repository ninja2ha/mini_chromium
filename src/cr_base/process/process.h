// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_H_
#define MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_H_

#include "cr_base/compiler_config.h"
#include "cr_base/compiler_specific.h"

#include "cr_base/base_export.h"
#include "cr_base/compiler_specific.h"
#include "cr_base/process/process_handle.h"
#include "cr_base/time/time.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/scoped_handle.h"
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
  // On Windows, this takes ownership of |handle|. On POSIX, this does not take
  // ownership of |handle|.
  explicit Process(ProcessHandle handle = kNullProcessHandle);

  Process(Process&& other);

  Process(const Process&) = delete;
  Process& operator=(const Process&) = delete;

  // The destructor does not terminate the process.
  ~Process();

  Process& operator=(Process&& other);

  // The result code that is used when a process is killed by a bad message.
  // Consistent with the value of RESULT_CODE_KILLED_BAD_MESSAGE in
  // content/public/common/result_codes.h.
  static constexpr int kResultCodeKilledBadMessage = 3;

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

  // Returns true if changing the priority of processes through `SetPriority()`
  // is possible.
  static bool CanSetPriority();

  // Terminates the current process immediately with |exit_code|.
  static void TerminateCurrentProcessImmediately(int exit_code);

  // Returns true if this objects represents a valid process.
  bool IsValid() const;

  // Returns a handle for this process. There is no guarantee about when that
  // handle becomes invalid because this object retains ownership.
  ProcessHandle Handle() const;

  // Returns a second object that represents this process.
  Process Duplicate() const;

  // Relinquishes ownership of the handle and sets this to kNullProcessHandle.
  // The result may be a pseudo-handle, depending on the OS and value stored in
  // this.
  ProcessHandle Release() CR_WARN_UNUSED_RESULT;

  // Get the PID for this process.
  ProcessId Pid() const;

  // Get the creation time for this process. Since the Pid can be reused after a
  // process dies, it is useful to use both the Pid and the creation time to
  // uniquely identify a process.
  //
  // On Android, works only if |this| is the current process, as security
  // features prevent an application from getting data about other processes,
  // even if they belong to us. Otherwise, returns Time().
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
  // NOTE: |exit_code| is only used on OS_WIN.
  bool Terminate(int exit_code, bool wait) const;

#if defined(MINI_CHROMIUM_OS_WIN)
  enum class WaitExitStatus {
    PROCESS_EXITED,
    STOP_EVENT_SIGNALED,
    FAILED,
  };

  // Waits for the process to exit, or the specified |stop_event_handle| to be
  // set. Returns value indicating which event was set. The given |exit_code|
  // will be the exit code of the process.
  WaitExitStatus WaitForExitOrEvent(
      const cr::win::ScopedHandle& stop_event_handle,
      int* exit_code) const;
#endif  // defined(MINI_CHROMIUM_OS_WIN)

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

  // Indicates that the process has exited with the specified |exit_code|.
  // This should be called if process exit is observed outside of this class.
  // (i.e. Not because Terminate or WaitForExit, above, was called.)
  // Note that nothing prevents this being called multiple times for a dead
  // process though that should be avoided.
  void Exited(int exit_code) const;

  // The different priorities that a process can have.
  enum class Priority {
    kMinValue = 0,

    // The process does not contribute to content that is currently important
    // to the user. Lowest priority.
    kBestEffort = kMinValue,

    // The process contributes to content that is visible to the user, but the
    // work don't have significant performance or latency requirement, so it can
    // run in energy efficient manner. Moderate priority.
    kUserVisible,

    // The process contributes to content that is of the utmost importance to
    // the user, like producing audible content, or visible content in the
    // main frame. High priority.
    kUserBlocking,

    kMaxValue = kUserBlocking,
  };

  // Retrieves the priority of the process. Defaults to Priority::kUserBlocking
  // if the priority could not be retrieved.
  Priority GetPriority() const;

  // Sets the priority of the process process. Returns true if the priority was
  // changed, false otherwise.
  bool SetPriority(Priority priority);

  // Returns an integer representing the priority of a process. The meaning
  // of this value is OS dependent.
  int GetOSPriority() const;

#if defined(MINI_CHROMIUM_OS_LINUX)
  // Returns true if the process has any seccomp policy applied.
  bool IsSeccompSandboxed();
#endif  // defined(MINI_CHROMIUM_OS_LINUX)

 private:
  bool TerminateInternal(int exit_code, bool wait) const;
  bool WaitForExitWithTimeoutImpl(cr::ProcessHandle handle,
                                  int* exit_code,
                                  cr::TimeDelta timeout) const;

#if defined(MINI_CHROMIUM_OS_WIN)
  win::ScopedHandle process_;
#else
  ProcessHandle process_;
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
  bool is_current_process_;
#endif
};

CRBASE_EXPORT const char* ProcessPriorityToString(
    Process::Priority process_priority);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_PROCESS_PROCESS_H_
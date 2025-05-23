// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/process/process.h"

#include <windows.h>

///#include "crbase/debug/activity_tracker.h"
#include "crbase/logging.h"
#include "crbase/debug/immediate_crash.h"
#include "crbase/numerics/safe_conversions.h"
///#include "crbase/process/kill.h"
#include "crbase/threading/thread_restrictions.h"
#include "crbase/win/windows_version.h"

namespace {

DWORD kBasicProcessAccess =
  PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | SYNCHRONIZE;

} // namespace

namespace cr {

Process::Process(ProcessHandle handle)
    : process_(handle), is_current_process_(false) {
  CR_CHECK(handle != ::GetCurrentProcess());
}

Process::Process(Process&& other)
    : process_(other.process_.Take()),
      is_current_process_(other.is_current_process_) {
  other.Close();
}

Process::~Process() {
}

Process& Process::operator=(Process&& other) {
  CR_DCHECK(this != &other);
  process_.Set(other.process_.Take());
  is_current_process_ = other.is_current_process_;
  other.Close();
  return *this;
}

// static
Process Process::Current() {
  Process process;
  process.is_current_process_ = true;
  return process;
}

// static
Process Process::Open(ProcessId pid) {
  return Process(::OpenProcess(kBasicProcessAccess, FALSE, pid));
}

// static
Process Process::OpenWithExtraPrivileges(ProcessId pid) {
  DWORD access = kBasicProcessAccess | PROCESS_DUP_HANDLE | PROCESS_VM_READ;
  return Process(::OpenProcess(access, FALSE, pid));
}

// static
Process Process::OpenWithAccess(ProcessId pid, DWORD desired_access) {
  return Process(::OpenProcess(desired_access, FALSE, pid));
}

// static
Process Process::DeprecatedGetProcessFromHandle(ProcessHandle handle) {
  CR_DCHECK(handle != ::GetCurrentProcess());
  ProcessHandle out_handle;
  if (!::DuplicateHandle(GetCurrentProcess(), handle,
                         GetCurrentProcess(), &out_handle,
                         0, FALSE, DUPLICATE_SAME_ACCESS)) {
    return Process();
  }
  return Process(out_handle);
}

// static
bool Process::CanBackgroundProcesses() {
  return true;
}

// static
void Process::TerminateCurrentProcessImmediately(int exit_code) {
  ::TerminateProcess(GetCurrentProcess(), exit_code);
  // There is some ambiguity over whether the call above can return. Rather than
  // hitting confusing crashes later on we should crash right here.
  CR_IMMEDIATE_CRASH();
}

bool Process::IsValid() const {
  return process_.IsValid() || is_current();
}

ProcessHandle Process::Handle() const {
  return is_current_process_ ? GetCurrentProcess() : process_.Get();
}

Process Process::Duplicate() const {
  if (is_current())
    return Current();

  ProcessHandle out_handle;
  if (!IsValid() || !::DuplicateHandle(GetCurrentProcess(),
                                       Handle(),
                                       GetCurrentProcess(),
                                       &out_handle,
                                       0,
                                       FALSE,
                                       DUPLICATE_SAME_ACCESS)) {
    return Process();
  }
  return Process(out_handle);
}

ProcessId Process::Pid() const {
  CR_DCHECK(IsValid());
  return GetProcId(Handle());
}

Time Process::CreationTime() const {
  FILETIME creation_time = {};
  FILETIME ignore1 = {};
  FILETIME ignore2 = {};
  FILETIME ignore3 = {};
  if (!::GetProcessTimes(Handle(), &creation_time, &ignore1, &ignore2,
                         &ignore3)) {
    return Time();
  }
  return Time::FromFileTime(creation_time);
}

bool Process::is_current() const {
  return is_current_process_;
}

void Process::Close() {
  is_current_process_ = false;
  if (!process_.IsValid())
    return;

  process_.Close();
}

bool Process::Terminate(int exit_code, bool wait) const {
  constexpr DWORD kWaitMs = 60 * 1000;

  // exit_code cannot be implemented.
  bool result = (::TerminateProcess(Handle(), exit_code) != FALSE);
  if (result && wait) {
    // The process may not end immediately due to pending I/O
    if (::WaitForSingleObject(Handle(), kWaitMs) != WAIT_OBJECT_0)
      CR_DPLOG(Error) << "Error waiting for process exit";
  } else if (!result) {
    CR_DPLOG(Error) << "Unable to terminate process";
  }
  return result;
}

bool Process::WaitForExit(int* exit_code) const {
  return WaitForExitWithTimeout(TimeDelta::FromMilliseconds(INFINITE),
                                exit_code);
}

bool Process::WaitForExitWithTimeout(TimeDelta timeout, int* exit_code) const {
  // Limit timeout to INFINITE.
  DWORD timeout_ms = saturated_cast<DWORD>(timeout.InMilliseconds());
  if (::WaitForSingleObject(Handle(), timeout_ms) != WAIT_OBJECT_0)
    return false;

  DWORD temp_code;  // Don't clobber out-parameters in case of failure.
  if (!::GetExitCodeProcess(Handle(), &temp_code))
    return false;

  if (exit_code)
    *exit_code = temp_code;
  return true;
}

bool Process::IsProcessBackgrounded() const {
  CR_DCHECK(IsValid());
  DWORD priority = GetPriority();
  if (priority == 0)
    return false;  // Failure case.
  return ((priority == BELOW_NORMAL_PRIORITY_CLASS) ||
          (priority == IDLE_PRIORITY_CLASS));
}

bool Process::SetProcessBackgrounded(bool value) {
  CR_DCHECK(IsValid());
  // Vista and above introduce a real background mode, which not only
  // sets the priority class on the threads but also on the IO generated
  // by it. Unfortunately it can only be set for the calling process.
  DWORD priority;
  if (win::GetVersion() >= win::Version::VISTA && is_current()) {
    priority = value ? PROCESS_MODE_BACKGROUND_BEGIN :
                       PROCESS_MODE_BACKGROUND_END;
  } else {
    priority = value ? IDLE_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
  }

  return (::SetPriorityClass(Handle(), priority) != 0);
}

int Process::GetPriority() const {
  CR_DCHECK(IsValid());
  return ::GetPriorityClass(Handle());
}

}  // namespace cr
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/platform_thread.h"

#include <stddef.h>

#include "crbase/logging/logging.h"
#include "crbase/debug/alias.h"
#include "crbase/debug/immediate_crash.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/strings/utf_string_conversions.h"
///#include "base/threading/scoped_blocking_call.h"
///#include "base/threading/scoped_thread_priority.h"
#include "crbase/threading/thread_id_name_manager.h"
///#include "base/threading/thread_restrictions.h"
#include "crbase/time/time_override.h"
#include "crbase/win/scoped_handle.h"
#include "crbase/win/windows_types.h"
#include "crbase/win/windows_version.h"
#include "crbase/win/win_api_helper.h"
#include "crbuild/build_config.h"

namespace cr {

namespace {

// The information on how to set the thread name comes from
// a MSDN article: http://msdn2.microsoft.com/en-us/library/xcb2z8hs.aspx
const DWORD kVCThreadNameException = 0x406D1388;

typedef struct tagTHREADNAME_INFO {
  DWORD dwType;  // Must be 0x1000.
  LPCSTR szName;  // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;

// The SetThreadDescription API was brought in version 1607 of Windows 10.
typedef HRESULT(WINAPI* SetThreadDescription)(HANDLE hThread,
                                              PCWSTR lpThreadDescription);

// This function has try handling, so it is separated out of its caller.
void SetNameInternal(PlatformThreadId thread_id, const char* name) {
#if defined(_MSC_VER)
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = name;
  info.dwThreadID = thread_id;
  info.dwFlags = 0;

  __try {
    RaiseException(kVCThreadNameException, 0, sizeof(info)/sizeof(DWORD),
                   reinterpret_cast<DWORD_PTR*>(&info));
  } __except(EXCEPTION_CONTINUE_EXECUTION) {
  }
#else
  UNREFERENCED_PARAMETER(thread_id);
  UNREFERENCED_PARAMETER(name);
#endif
}

struct ThreadParams {
  PlatformThread::Delegate* delegate;
  bool joinable;
  ThreadPriority priority;
};

DWORD __stdcall ThreadFunc(void* params) {
  ThreadParams* thread_params = static_cast<ThreadParams*>(params);
  PlatformThread::Delegate* delegate = thread_params->delegate;
  ///if (!thread_params->joinable)
  ///  base::ThreadRestrictions::SetSingletonAllowed(false);

  ///if (thread_params->priority != ThreadPriority::NORMAL)
  ///  PlatformThread::SetCurrentThreadPriority(thread_params->priority);

  // Retrieve a copy of the thread handle to use as the key in the
  // thread name mapping.
  PlatformThreadHandle::Handle platform_handle;
  BOOL did_dup = DuplicateHandle(GetCurrentProcess(),
                                GetCurrentThread(),
                                GetCurrentProcess(),
                                &platform_handle,
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS);

  win::ScopedHandle scoped_platform_handle;

  if (did_dup) {
    scoped_platform_handle.Set(platform_handle);
    ThreadIdNameManager::GetInstance()->RegisterThread(
        scoped_platform_handle.Get(),
        PlatformThread::CurrentId());
  }

  delete thread_params;
  delegate->ThreadMain();

  if (did_dup) {
    ThreadIdNameManager::GetInstance()->RemoveName(
        scoped_platform_handle.Get(),
        PlatformThread::CurrentId());
  }

  // Ensure thread priority is at least NORMAL before initiating thread
  // destruction. Thread destruction on Windows holds the LdrLock while
  // performing TLS destruction which causes hangs if performed at background
  // priority (priority inversion) (see: http://crbug.com/1096203).
  ///if (PlatformThread::GetCurrentThreadPriority() < ThreadPriority::NORMAL)
  ///  PlatformThread::SetCurrentThreadPriority(ThreadPriority::NORMAL);

  return 0;
}

// CreateThreadInternal() matches PlatformThread::CreateWithPriority(), except
// that |out_thread_handle| may be nullptr, in which case a non-joinable thread
// is created.
bool CreateThreadInternal(size_t stack_size,
                          PlatformThread::Delegate* delegate,
                          PlatformThreadHandle* out_thread_handle,
                          ThreadPriority priority) {
  unsigned int flags = 0;
  if (stack_size > 0) {
    flags = STACK_SIZE_PARAM_IS_A_RESERVATION;
#if defined(MINI_CHROMIUM_ARCH_CPU_32_BITS)
  } else {
    // The process stack size is increased to give spaces to |RendererMain| in
    // |chrome/BUILD.gn|, but keep the default stack size of other threads to
    // 1MB for the address space pressure.
    flags = STACK_SIZE_PARAM_IS_A_RESERVATION;
    static BOOL is_wow64 = -1;
    if (is_wow64 == -1 && !IsWow64Process(GetCurrentProcess(), &is_wow64))
      is_wow64 = FALSE;
    // When is_wow64 is set that means we are running on 64-bit Windows and we
    // get 4 GiB of address space. In that situation we can afford to use 1 MiB
    // of address space for stacks. When running on 32-bit Windows we only get
    // 2 GiB of address space so we need to conserve. Typically stack usage on
    // these threads is only about 100 KiB.
    if (is_wow64)
      stack_size = 1024 * 1024;
    else
      stack_size = 512 * 1024;
#endif
  }

  ThreadParams* params = new ThreadParams;
  params->delegate = delegate;
  params->joinable = out_thread_handle != nullptr;
  params->priority = priority;

  // Using CreateThread here vs _beginthreadex makes thread creation a bit
  // faster and doesn't require the loader lock to be available.  Our code will
  // have to work running on CreateThread() threads anyway, since we run code on
  // the Windows thread pool, etc.  For some background on the difference:
  // http://www.microsoft.com/msj/1099/win32/win321099.aspx
  void* thread_handle =
      ::CreateThread(nullptr, stack_size, ThreadFunc, params, flags, nullptr);

  if (!thread_handle) {
    DWORD last_error = ::GetLastError();

    switch (last_error) {
      case ERROR_NOT_ENOUGH_MEMORY:
      case ERROR_OUTOFMEMORY:
      case ERROR_COMMITMENT_LIMIT:
        cr::debug::ImmediateCrashBecauseOutofMemory(stack_size);
        break;

      default:
        ///static auto* last_error_crash_key = debug::AllocateCrashKeyString(
        ///    "create_thread_last_error", debug::CrashKeySize::Size32);
        ///debug::SetCrashKeyString(last_error_crash_key,
        ///                         base::NumberToString(last_error));
        break;
    }

    delete params;
    return false;
  }

  if (out_thread_handle)
    *out_thread_handle = PlatformThreadHandle(thread_handle);
  else
    CloseHandle(thread_handle);
  return true;
}

}  // namespace

// static
PlatformThreadId PlatformThread::CurrentId() {
  return ::GetCurrentThreadId();
}

// static
PlatformThreadRef PlatformThread::CurrentRef() {
  return PlatformThreadRef(::GetCurrentThreadId());
}

// static
PlatformThreadHandle PlatformThread::CurrentHandle() {
  return PlatformThreadHandle(::GetCurrentThread());
}

// static
void PlatformThread::YieldCurrentThread() {
  ::Sleep(0);
}

// static
void PlatformThread::Sleep(TimeDelta duration) {
  // When measured with a high resolution clock, Sleep() sometimes returns much
  // too early. We may need to call it repeatedly to get the desired duration.
  // PlatformThread::Sleep doesn't support mock-time, so this always uses
  // real-time.
  const TimeTicks end = subtle::TimeTicksNowIgnoringOverride() + duration;
  for (TimeTicks now = subtle::TimeTicksNowIgnoringOverride(); now < end;
       now = subtle::TimeTicksNowIgnoringOverride()) {
    ::Sleep(static_cast<DWORD>((end - now).InMillisecondsRoundedUp()));
  }
}

// static
void PlatformThread::SetName(const std::string& name) {
  ThreadIdNameManager::GetInstance()->SetName(name);

  // The SetThreadDescription API works even if no debugger is attached.
  static auto set_thread_description_func =
      reinterpret_cast<SetThreadDescription>(::GetProcAddress(
          win::GetKernel32Module(), "SetThreadDescription"));
  if (set_thread_description_func) {
    set_thread_description_func(::GetCurrentThread(),
                                cr::UTF8ToWide(name).c_str());
  }

  // The debugger needs to be around to catch the name in the exception.  If
  // there isn't a debugger, we are just needlessly throwing an exception.
  if (!::IsDebuggerPresent())
    return;

  SetNameInternal(CurrentId(), name.c_str());
}

// static
const char* PlatformThread::GetName() {
  return ThreadIdNameManager::GetInstance()->GetName(CurrentId());
}

// static
bool PlatformThread::CreateWithPriority(size_t stack_size, Delegate* delegate,
                                        PlatformThreadHandle* thread_handle,
                                        ThreadPriority priority) {
  CR_DCHECK(thread_handle);
  return CreateThreadInternal(stack_size, delegate, thread_handle, priority);
}

// static
bool PlatformThread::CreateNonJoinable(size_t stack_size, Delegate* delegate) {
  return CreateNonJoinableWithPriority(stack_size, delegate,
                                       ThreadPriority::NORMAL);
}

// static
bool PlatformThread::CreateNonJoinableWithPriority(size_t stack_size,
                                                   Delegate* delegate,
                                                   ThreadPriority priority) {
  return CreateThreadInternal(stack_size, delegate, nullptr /* non-joinable */,
                              priority);
}

// static
void PlatformThread::Join(PlatformThreadHandle thread_handle) {
  CR_DCHECK(thread_handle.platform_handle());

  DWORD thread_id = 0;
  thread_id = ::GetThreadId(thread_handle.platform_handle());
  DWORD last_error = 0;
  if (!thread_id)
    last_error = ::GetLastError();

  // Record information about the exiting thread in case joining hangs.
  cr::debug::Alias(&thread_id);
  cr::debug::Alias(&last_error);

  ///base::internal::ScopedBlockingCallWithBaseSyncPrimitives scoped_blocking_call(
  ///    FROM_HERE, base::BlockingType::MAY_BLOCK);

  // Wait for the thread to exit.  It should already have terminated but make
  // sure this assumption is valid.
  CR_CHECK(WAIT_OBJECT_0 ==
           WaitForSingleObject(thread_handle.platform_handle(), INFINITE));
  CloseHandle(thread_handle.platform_handle());
}

// static
void PlatformThread::Detach(PlatformThreadHandle thread_handle) {
  CloseHandle(thread_handle.platform_handle());
}

// static
size_t PlatformThread::GetDefaultThreadStackSize() {
  return 0;
}

}  // namespace cr
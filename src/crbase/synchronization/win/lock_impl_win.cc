// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 61.0.3163.141

#include "crbase/synchronization/lock_impl.h"

#include "crbase/logging/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/win/windows_types.h"
#include "crbase/win/win_api_helper.h"

namespace cr {
namespace internal {

namespace {

using LockImplFunction =  BOOL(WINAPI*)(LPVOID);
struct LockFunctionMember {
  LockImplFunction InitLock = nullptr;
  LockImplFunction TryAcquireLock = nullptr;
  LockImplFunction AcquireLock = nullptr;
  LockImplFunction ReleaseLock = nullptr;
  LockImplFunction UnitLock = nullptr;
};

BOOL WINAPI InitializeCriticalSection(LPVOID handle) {
  // The second parameter is the spin count, for short-held locks it avoid the
  // contending thread from going to sleep which helps performance greatly.
  return ::InitializeCriticalSectionAndSpinCount(
      reinterpret_cast<LPCRITICAL_SECTION>(handle), 2000);
}

LockFunctionMember* lock_funcs_() {
  static cr::NoDestructor<LockFunctionMember> funcs([]{
    HMODULE kernel32 = cr::win::GetKernel32Module();
    LockFunctionMember member;
    member.InitLock = nullptr;
    member.UnitLock = nullptr;
    member.TryAcquireLock = reinterpret_cast<LockImplFunction>(
        GetProcAddress(kernel32, "TryAcquireSRWLockExclusive"));
    member.AcquireLock = reinterpret_cast<LockImplFunction>(
        GetProcAddress(kernel32, "AcquireSRWLockExclusive"));
    member.ReleaseLock = reinterpret_cast<LockImplFunction>(
        GetProcAddress(kernel32, "ReleaseSRWLockExclusive"));
    if (member.TryAcquireLock && member.AcquireLock && member.ReleaseLock)
      return member;

    member.InitLock = reinterpret_cast<LockImplFunction>(
        &cr::internal::InitializeCriticalSection);
    member.TryAcquireLock = reinterpret_cast<LockImplFunction>(
        &TryEnterCriticalSection);
    member.AcquireLock = reinterpret_cast<LockImplFunction>(
        &EnterCriticalSection);
    member.ReleaseLock = reinterpret_cast<LockImplFunction>(
        &LeaveCriticalSection);
    member.UnitLock = reinterpret_cast<LockImplFunction>(
        &DeleteCriticalSection);
    return member;
  }());
  return funcs.get();
}
  
}  // namespace

LockImpl::LockImpl() : native_handle_({0}) {
  if (lock_funcs_()->InitLock)
    lock_funcs_()->InitLock(&native_handle_);
}

LockImpl::~LockImpl() {
  if (lock_funcs_()->UnitLock)
    lock_funcs_()->UnitLock(&native_handle_);
}

bool LockImpl::Try() {
  CR_DCHECK(lock_funcs_()->TryAcquireLock);
  return !!lock_funcs_()->TryAcquireLock(&native_handle_);
}

void LockImpl::Lock() {
  CR_DCHECK(lock_funcs_()->AcquireLock);
  lock_funcs_()->AcquireLock(&native_handle_);
}

void LockImpl::Unlock() {
  CR_DCHECK(lock_funcs_()->ReleaseLock);
  lock_funcs_()->ReleaseLock(&native_handle_);
}

}  // namespace internal
}  // namespace cr
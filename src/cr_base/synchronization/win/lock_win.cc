// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 61.0.3163.141

#include "cr_base/synchronization/lock.h"

// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>

#include "cr_base/logging/logging.h"

namespace cr {
namespace internal {

LockImpl::LockImpl() {
  ::InitializeCriticalSectionAndSpinCount(
      cr::win::AsCriticalSestion(&native_handle_), 2000);
}

LockImpl::~LockImpl() {
  ::DeleteCriticalSection(
      cr::win::AsCriticalSestion(&native_handle_));
}

bool LockImpl::Try() {
  return !!::TryEnterCriticalSection(
      cr::win::AsCriticalSestion(&native_handle_));
}

void LockImpl::Lock() {
  ::EnterCriticalSection(
      cr::win::AsCriticalSestion(&native_handle_));
}

void LockImpl::Unlock() {
  ::LeaveCriticalSection(
      cr::win::AsCriticalSestion(&native_handle_));
}

}  // namespace internal
}  // namespace cr
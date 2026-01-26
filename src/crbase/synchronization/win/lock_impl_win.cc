// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 61.0.3163.141

#include "crbase/synchronization/lock_impl.h"

#include "crbase/logging/logging.h"
#include "crbase/win/windows_types.h"

namespace cr {
namespace internal {

LockImpl::LockImpl() {
  ::InitializeCriticalSectionAndSpinCount(&native_handle_, 2000);
}

LockImpl::~LockImpl() {
  ::DeleteCriticalSection(&native_handle_);
}

bool LockImpl::Try() {
  return !!::TryEnterCriticalSection(&native_handle_);
}

void LockImpl::Lock() {
  ::EnterCriticalSection(&native_handle_);
}

void LockImpl::Unlock() {
  ::LeaveCriticalSection(&native_handle_);
}

}  // namespace internal
}  // namespace cr
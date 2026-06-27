// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/threading/platform_thread.h"

#include <memory>

#include "cr_base/time/time.h"

namespace cr {

// static
void PlatformThread::SetCurrentThreadPriority(ThreadPriority priority) {
    SetCurrentThreadPriorityImpl(priority);
}

TimeDelta PlatformThread::Delegate::GetRealtimePeriod() {
  return TimeDelta();
}

}  // namespace cr
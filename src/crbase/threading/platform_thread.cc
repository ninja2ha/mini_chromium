// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/platform_thread.h"

#include <memory>

namespace cr {

TimeDelta PlatformThread::GetRealtimePeriod(Delegate* delegate) {
  if (delegate)
    return delegate->GetRealtimePeriod();
  return TimeDelta();
}

TimeDelta PlatformThread::Delegate::GetRealtimePeriod() {
  return TimeDelta();
}

}  // namespace cr
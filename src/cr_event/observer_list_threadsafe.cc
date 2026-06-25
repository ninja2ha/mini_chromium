// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_event/observer_list_threadsafe.h"

#include "cr_base/memory/no_destructor.h"

namespace cr {
namespace internal {

// static 
ThreadLocalPointer<const ObserverListThreadSafeBase::NotificationDataBase>*
ObserverListThreadSafeBase::tls_current_notification() {
  static cr::NoDestructor<ThreadLocalPointer<const NotificationDataBase>> tls;
  return tls.get();
}

}  // namespace internal
}  // namespace cr
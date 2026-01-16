// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/threading/thread_local_storage.h"

#include "crbase/logging/logging.h"

namespace cr {

namespace internal {

bool PlatformThreadLocalStorage::AllocTLS(TLSKey* key) {
  return !pthread_key_create(key,
      cr::internal::PlatformThreadLocalStorage::OnThreadExit);
}

void PlatformThreadLocalStorage::FreeTLS(TLSKey key) {
  int ret = pthread_key_delete(key);
  CR_DCHECK(ret == 0);
}

void PlatformThreadLocalStorage::SetTLSValue(TLSKey key, void* value) {
  int ret = pthread_setspecific(key, value);
  CR_DCHECK(ret == 0);
}

}  // namespace internal

}  // namespace cr

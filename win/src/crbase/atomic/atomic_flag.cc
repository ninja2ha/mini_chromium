// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/atomic/atomic_flag.h"

#include "crbase/logging.h"

namespace cr {

AtomicFlag::AtomicFlag() {
  // It doesn't matter where the AtomicFlag is built so long as it's always
  // Set() from the same sequence after. Note: the sequencing requirements are
  // necessary for IsSet()'s callers to know which sequence's memory operations
  // they are synchronized with.
}

AtomicFlag::~AtomicFlag() = default;

void AtomicFlag::Set() {
  flag_.store(1, std::memory_order_release);
}

}  // namespace cr
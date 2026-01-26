// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase_runtime/threading/sequence_local_storage_slot.h"

#include <limits>

#include "crbase/atomic/atomic_sequence_num.h"
#include "crbase/logging/logging.h"

namespace cr {
namespace internal {

namespace {
AtomicSequenceNumber g_sequence_local_storage_slot_generator;
}  // namespace

AtomicSequenceNumber::IntType GetNextSequenceLocalStorageSlotNumber() {
  auto slot_id = g_sequence_local_storage_slot_generator.GetNext();
  ///CR_DCHECK(slot_id < std::numeric_limits<int>::max());
  return slot_id;
}

}  // namespace internal

}  // namespace cr
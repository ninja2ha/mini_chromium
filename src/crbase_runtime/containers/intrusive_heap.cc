// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/containers/intrusive_heap.h"

#include "crbase/logging/logging.h"
#include "crbase/memory/ptr_util.h"

namespace cr {

////////////////////////////////////////////////////////////////////////////////
// HeapHandle

// static
HeapHandle HeapHandle::Invalid() {
  return HeapHandle();
}

////////////////////////////////////////////////////////////////////////////////
// InternalHeapHandleStorage

InternalHeapHandleStorage::InternalHeapHandleStorage()
    : handle_(new HeapHandle()) {}

InternalHeapHandleStorage::InternalHeapHandleStorage(
    InternalHeapHandleStorage&& other) noexcept
    : handle_(std::move(other.handle_)) {
  CR_DCHECK(intrusive_heap::IsInvalid(other.handle_));
}

InternalHeapHandleStorage::~InternalHeapHandleStorage() = default;

InternalHeapHandleStorage& InternalHeapHandleStorage::operator=(
    InternalHeapHandleStorage&& other) noexcept {
  handle_ = std::move(other.handle_);
  CR_DCHECK(intrusive_heap::IsInvalid(other.handle_));
  return *this;
}

void InternalHeapHandleStorage::swap(
    InternalHeapHandleStorage& other) noexcept {
  std::swap(handle_, other.handle_);
}

}  // namespace cr

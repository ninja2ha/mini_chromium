// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/memory/weak_ptr.h"

namespace cr {
namespace internal {

WeakReference::Flag::Flag() {
  // Flags only become bound when checked for validity, or invalidated,
  // so that we can check that later validity/invalidation operations on
  // the same Flag take place on the same sequenced thread.
}

void WeakReference::Flag::Invalidate() {
  // The flag being invalidated with a single ref implies that there are no
  // weak pointers in existence. Allow deletion on other thread in this case.
  invalidated_.Set();
}

bool WeakReference::Flag::IsValid() const {
  // WeakPtrs must be checked on the same sequenced thread.
  return !invalidated_.IsSet();
}

bool WeakReference::Flag::MaybeValid() const {
  return !invalidated_.IsSet();
}

void WeakReference::Flag::DetachFromSequence() {
}

WeakReference::Flag::~Flag() = default;

WeakReference::WeakReference() = default;

WeakReference::WeakReference(const RefPtr<Flag>& flag) : flag_(flag) {}

WeakReference::~WeakReference() = default;

WeakReference::WeakReference(WeakReference&& other) noexcept = default;

WeakReference::WeakReference(const WeakReference& other) = default;

bool WeakReference::IsValid() const {
  return flag_ && flag_->IsValid();
}

bool WeakReference::MaybeValid() const {
  return flag_ && flag_->MaybeValid();
}

WeakReferenceOwner::WeakReferenceOwner()
    : flag_(MakeRefCounted<WeakReference::Flag>()) {}

WeakReferenceOwner::~WeakReferenceOwner() {
  flag_->Invalidate();
}

WeakReference WeakReferenceOwner::GetRef() const {
  // If we hold the last reference to the Flag then detach the SequenceChecker.
  if (!HasRefs())
    flag_->DetachFromSequence();

  return WeakReference(flag_);
}

void WeakReferenceOwner::Invalidate() {
  flag_->Invalidate();
  flag_ = MakeRefCounted<WeakReference::Flag>();
}

WeakPtrBase::WeakPtrBase() : ptr_(0) {}

WeakPtrBase::~WeakPtrBase() = default;

WeakPtrBase::WeakPtrBase(const WeakReference& ref, uintptr_t ptr)
    : ref_(ref), ptr_(ptr) {
  CR_DCHECK(ptr_);
}

WeakPtrFactoryBase::WeakPtrFactoryBase(uintptr_t ptr) : ptr_(ptr) {
  CR_DCHECK(ptr_);
}

WeakPtrFactoryBase::~WeakPtrFactoryBase() {
  ptr_ = 0;
}

}  // namespace internal
}  // namespace cr
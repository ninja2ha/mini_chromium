// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/memory/weak_ptr.h"

///#if DCHECK_IS_ON()
///#include "base/debug/stack_trace.h"
///#endif

namespace cr {
namespace internal {

WeakReference::Flag::Flag() {
  // Flags only become bound when checked for validity, or invalidated,
  // so that we can check that later validity/invalidation operations on
  // the same Flag take place on the same sequenced thread.
  ///DETACH_FROM_SEQUENCE(sequence_checker_);
}

void WeakReference::Flag::Invalidate() {
  // The flag being invalidated with a single ref implies that there are no
  // weak pointers in existence. Allow deletion on other thread in this case.
#if CR_DCHECK_IS_ON()
  ///std::unique_ptr<debug::StackTrace> bound_at;
  ///CR_DCHECK(sequence_checker_.CalledOnValidSequence(&bound_at) || HasOneRef())
  ///    << "WeakPtrs must be invalidated on the same sequenced thread as where "
  ///    << "they are bound.\n"
  ///    << (bound_at ? "This was bound at:\n" + bound_at->ToString() : "")
  ///    << "Check failed at:";
  CR_DCHECK(sequence_checker_.CalledOnValidSequence() || HasOneRef())
      << "WeakPtrs must be invalidated on the same sequenced thread.";
#endif
  invalidated_.Set();
}

bool WeakReference::Flag::IsValid() const {
  // WeakPtrs must be checked on the same sequenced thread.
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return !invalidated_.IsSet();
}

bool WeakReference::Flag::MaybeValid() const {
  return !invalidated_.IsSet();
}

void WeakReference::Flag::DetachFromSequence() {
  CR_DETACH_FROM_SEQUENCE(sequence_checker_);
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

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CREVENT_MEMORY_REF_COUNTED_DELETE_ON_SEQUENCE_H_
#define MINI_CHROMIUM_SRC_CREVENT_MEMORY_REF_COUNTED_DELETE_ON_SEQUENCE_H_

#include <utility>

#include "cr_base/logging/logging.h"
#include "cr_base/location.h"
#include "cr_base/memory/ref_counted.h"

#include "cr_event/task/sequenced_task_runner.h"

namespace cr {

// RefCountedDeleteOnSequence is similar to RefCountedThreadSafe, and ensures
// that the object will be deleted on a specified sequence.
//
// Sample usage:
// class Foo : public RefCountedDeleteOnSequence<Foo> {
//
//   Foo(scoped_refptr<SequencedTaskRunner> task_runner)
//       : RefCountedDeleteOnSequence<Foo>(std::move(task_runner)) {}
//   ...
//  private:
//   friend class RefCountedDeleteOnSequence<Foo>;
//   friend class DeleteHelper<Foo>;
//
//   ~Foo();
// };
template <class T>
class RefCountedDeleteOnSequence : public subtle::RefCountedThreadSafeBase {
 public:
  static constexpr subtle::StartRefCountFromZeroTag kRefCountPreference =
      subtle::kStartRefCountFromZeroTag;

  // A SequencedTaskRunner for the current sequence can be acquired by calling
  // SequencedTaskRunnerHandle::Get().
  explicit RefCountedDeleteOnSequence(
      RefPtr<SequencedTaskRunner> owning_task_runner)
      : subtle::RefCountedThreadSafeBase(T::kRefCountPreference),
        owning_task_runner_(std::move(owning_task_runner)) {
    CR_DCHECK(owning_task_runner_);
  }

  void AddRef() const { AddRefImpl(T::kRefCountPreference); }

  void Release() const {
    if (subtle::RefCountedThreadSafeBase::Release())
      DestructOnSequence();
  }

 protected:
  friend class DeleteHelper<RefCountedDeleteOnSequence>;
  ~RefCountedDeleteOnSequence() = default;

  SequencedTaskRunner* owning_task_runner() {
    return owning_task_runner_.get();
  }
  const SequencedTaskRunner* owning_task_runner() const {
    return owning_task_runner_.get();
  }

 private:
  void DestructOnSequence() const {
    const T* t = static_cast<const T*>(this);
    if (owning_task_runner_->RunsTasksInCurrentSequence())
      delete t;
    else
      owning_task_runner_->DeleteSoon(CR_FROM_HERE, t);
  }

  void AddRefImpl(subtle::StartRefCountFromZeroTag) const {
    subtle::RefCountedThreadSafeBase::AddRef();
  }

  void AddRefImpl(subtle::StartRefCountFromOneTag) const {
    subtle::RefCountedThreadSafeBase::AddRefWithCheck();
  }

  const RefPtr<SequencedTaskRunner> owning_task_runner_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CREVENT_MEMORY_REF_COUNTED_DELETE_ON_SEQUENCE_H_

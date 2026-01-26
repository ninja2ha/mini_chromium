// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_INTERNAL_OBSERVER_LIST_TYPES_H_
#define MINI_CHROMIUM_SRC_CRBASE_INTERNAL_OBSERVER_LIST_TYPES_H_

#include "crbase/base_export.h"
#include "crbase/memory/weak_ptr.h"

namespace cr {
namespace internal {
class CheckedObserverAdapter;
}  // namespace internal

// A CheckedObserver serves as a base class for an observer interface designed
// to be used with base::ObserverList. It helps detect potential use-after-free
// issues that can occur when observers fail to remove themselves from an
// observer list upon destruction.
//
// A CheckedObserver will CHECK() if an ObserverList iteration is attempted over
// a destroyed Observer.
//
// Note that a CheckedObserver subclass must be deleted on the same thread as
// the ObserverList(s) it is added to. This is DCHECK()ed via WeakPtr.
class CRBASE_EXPORT CheckedObserver {
 public:
  CheckedObserver();
  CheckedObserver(const CheckedObserver&) = delete;
  CheckedObserver& operator=(const CheckedObserver&) = delete;

 protected:
  virtual ~CheckedObserver();

  // Returns whether |this| is in any ObserverList. Subclasses can CHECK() this
  // in their destructor to obtain a nicer stacktrace.
  bool IsInObserverList() const;

 private:
  friend class internal::CheckedObserverAdapter;

  // Must be mutable to allow ObserverList<const Foo>.
  mutable WeakPtrFactory<CheckedObserver> factory_{this};
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_INTERNAL_OBSERVER_LIST_TYPES_H_
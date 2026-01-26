// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/internal/observer_list_internal.h"

namespace cr {
namespace internal {

CheckedObserverAdapter::CheckedObserverAdapter(const CheckedObserver* observer)
    : weak_ptr_(observer->factory_.GetWeakPtr()) {}

CheckedObserverAdapter::CheckedObserverAdapter(CheckedObserverAdapter&& other) =
    default;
CheckedObserverAdapter& CheckedObserverAdapter::operator=(
    CheckedObserverAdapter&& other) = default;
CheckedObserverAdapter::~CheckedObserverAdapter() = default;

}  // namespace internal
}  // namespace cr
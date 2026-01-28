// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_
#define MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_

#include <stack>

#include "crbase/containers/circular_deque.h"

namespace cr {

template <class T, class Container = circular_deque<T>>
using Stack = std::stack<T, Container>;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_
#define MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_

#include <stack>

#include "crbase/containers/circular_deque.h"

namespace cr {

// Provides a definition of base::stack that's like std::stack but uses a
// cr::circular_deque instead of std::deque. Since std::stack is just a
// wrapper for an underlying type, we can just provide a typedef for it that
// defaults to the base circular_deque.
template <class T, class Container = circular_deque<T>>
using stack = std::stack<T, Container>;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_

// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_
#define MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_

#include <stack>

namespace cr {

template <typename T>
using Stack = std::stack<T>;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_STACK_H_
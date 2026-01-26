// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_UTIL_H_

#include <stdint.h>

namespace cr {

// TODO(crbug.com/817982): What we really need is for checked_math.h to be
// able to do checked arithmetic on pointers.
template <typename T>
inline uintptr_t get_uintptr(const T* t) {
  return reinterpret_cast<uintptr_t>(t);
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_CONTAINERS_UTIL_H_

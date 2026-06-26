// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_JSON_INTERNAL_JSON_COMMON_H_
#define MINI_CHROMIUM_SRC_CRBASE_JSON_INTERNAL_JSON_COMMON_H_

#include <stddef.h>

#include "cr_base/logging/logging.h"

namespace cr {
namespace internal {

// Chosen to support 99.9% of documents found in the wild late 2016.
// http://crbug.com/673263
const size_t kAbsoluteMaxDepth = 200;

// Simple class that checks for maximum recursion/stack overflow.
class StackMarker {
 public:
 StackMarker(const StackMarker&) = delete;
 StackMarker& operator=(const StackMarker&) = delete;

  StackMarker(size_t max_depth, size_t* depth)
      : max_depth_(max_depth), depth_(depth) {
    ++(*depth_);
    CR_DCHECK(*depth_ <= max_depth_);
  }
  ~StackMarker() { --(*depth_); }

  bool IsTooDeep() const { return *depth_ >= max_depth_; }

 private:
  const size_t max_depth_;
  size_t* const depth_;
};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_JSON_INTERNAL_JSON_COMMON_H_
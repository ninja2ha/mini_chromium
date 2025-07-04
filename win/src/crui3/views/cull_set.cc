// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/cull_set.h"

namespace crui {
namespace views {

CullSet::CullSet() {
}

CullSet::~CullSet() {
}

CullSet::CullSet(std::unique_ptr<std::unordered_set<intptr_t> > cull_set)
    : cull_set_(std::move(cull_set)) {
}

bool CullSet::ShouldPaint(const View* view) const {
  if (cull_set_)
    return (cull_set_->count(reinterpret_cast<intptr_t>(view)) > 0);

  return true;
}

}  // namespace views
}  // namespace crui

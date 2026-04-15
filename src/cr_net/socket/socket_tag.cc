// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/socket/socket_tag.h"

#include <tuple>

#include "cr_base/logging/logging.h"

namespace crnet {

bool SocketTag::operator<(const SocketTag& other) const {
  return false;
}

bool SocketTag::operator==(const SocketTag& other) const {
  return true;
}

void SocketTag::Apply(SocketDescriptor socket) const {
  CR_CHECK(false);
}

}  // namespace crnet
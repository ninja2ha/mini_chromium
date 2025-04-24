// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/socket/tcp/transport_client_socket.h"

#include "crbase/logging.h"

namespace crnet {

TransportClientSocket::TransportClientSocket() = default;
TransportClientSocket::~TransportClientSocket() = default;

bool TransportClientSocket::SetNoDelay(bool no_delay) {
  ///CR_NOTIMPLEMENTED();
  CR_NOTREACHED();
  return false;
}

bool TransportClientSocket::SetKeepAlive(bool enable, int delay_secs) {
  ///CR_NOTIMPLEMENTED();
  CR_NOTREACHED();
  return false;
}

}  // namespace crnet
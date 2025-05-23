// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/socket/tcp/stream_socket.h"

#include "crbase/logging.h"

namespace crnet {

void StreamSocket::SetBeforeConnectCallback(
    const BeforeConnectCallback& before_connect_callback) {
  CR_NOTREACHED();
}

StreamSocket::SocketMemoryStats::SocketMemoryStats()
    : total_size(0), buffer_size(0), cert_count(0), cert_size(0) {}

StreamSocket::SocketMemoryStats::~SocketMemoryStats() = default;

}  // namespace crnet
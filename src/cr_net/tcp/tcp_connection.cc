// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/tcp/tcp_connection.h"

#include <utility>

#include "cr_base/logging/logging.h"
#include "cr_net/socket/tcp/stream_socket.h"

namespace crnet {

TCPConnection::TCPConnection(TCPConnection::Id id, 
                             std::unique_ptr<StreamSocket> socket)
    : id_(id),
      socket_(std::move(socket)),
      read_buf_(cr::MakeRefCounted<cr::ReadIOBuffer>()),
      write_buf_(cr::MakeRefCounted<cr::QueuedWriteIOBuffer>()) {}

TCPConnection::~TCPConnection() = default;

}  // namespace crnet
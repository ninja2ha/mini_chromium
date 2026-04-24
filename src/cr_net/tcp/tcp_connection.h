// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CONNECTION_H_
#define MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CONNECTION_H_

#include <memory>
#include <string>
#include <queue>

#include "cr_base/containers/queue.h"
#include "cr_base/memory/ref_counted.h"
#include "cr_base/memory/weak_ptr.h"
#include "cr_event/io_buffer.h"
#include "cr_net/net_export.h"
#include "cr_build/build_config.h"

namespace crnet {

class StreamSocket;
class TCPServer;
class TCPClient;

// A container which has all information of an Tcp connection. It includes
// id, underlying socket, and pending read/write data.
class CRNET_EXPORT TCPConnection {
 public:
#if defined(MINI_CHROMIUM_ARCH_CPU_64_BITS)
  using Id = uint64_t;
#else
  using Id = uint32_t;
#endif

  TCPConnection(TCPConnection::Id id, std::unique_ptr<StreamSocket> socket);
  ~TCPConnection();

  Id id() const { return id_; }
  StreamSocket* socket() const { return socket_.get(); }

 private:
  friend class crnet::TCPServer;
  friend class crnet::TCPClient;

  cr::ReadIOBuffer* read_buf() const { return read_buf_.get(); }
  cr::QueuedWriteIOBuffer* write_buf() const { return write_buf_.get(); }

 private:
  const Id id_;
  const std::unique_ptr<StreamSocket> socket_;
  const cr::RefPtr<cr::ReadIOBuffer> read_buf_;
  const cr::RefPtr<cr::QueuedWriteIOBuffer> write_buf_;
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CONNECTION_H_
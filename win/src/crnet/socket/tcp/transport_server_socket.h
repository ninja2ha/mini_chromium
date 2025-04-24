// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TRANSPORT_SERVER_SOCKET_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TRANSPORT_SERVER_SOCKET_H_

#include <stdint.h>

#include <memory>
#include <string>

#include "crnet/base/completion_once_callback.h"
#include "crnet/base/net_export.h"

namespace crnet {

class IPEndPoint;
class StreamSocket;

class CRNET_EXPORT TransportServerSocket {
 public:
  TransportServerSocket(const TransportServerSocket&) = delete;
  TransportServerSocket& operator=(const TransportServerSocket&) = delete;

  TransportServerSocket();
  virtual ~TransportServerSocket();

  // Binds the socket and starts listening. Destroys the socket to stop
  // listening.
  virtual int Listen(const IPEndPoint& address, int backlog) = 0;

  // Binds the socket with address and port, and starts listening. It expects
  // a valid IPv4 or IPv6 address. Otherwise, it returns ERR_ADDRESS_INVALID.
  virtual int ListenWithAddressAndPort(const std::string& address_string,
                                       uint16_t port,
                                       int backlog);

  // Gets current address the socket is bound to.
  virtual int GetLocalAddress(IPEndPoint* address) const = 0;

  // Accepts connection. Callback is called when new connection is
  // accepted.
  virtual int Accept(std::unique_ptr<StreamSocket>* socket,
                     CompletionOnceCallback callback) = 0;
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TRANSPORT_SERVER_SOCKET_H_
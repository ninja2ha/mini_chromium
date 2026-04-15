// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/socket/tcp/transport_server_socket.h"

#include "cr_net/base/ip_address.h"
#include "cr_net/base/ip_endpoint.h"
#include "cr_net/base/net_errors.h"

namespace crnet {

TransportServerSocket::TransportServerSocket() = default;

TransportServerSocket::~TransportServerSocket() = default;

int TransportServerSocket::ListenWithAddressAndPort(
    const std::string& address_string,
    uint16_t port,
    int backlog) {
  IPAddress ip_address;
  if (!ip_address.AssignFromIPLiteral(address_string)) {
    return ERR_ADDRESS_INVALID;
  }

  return Listen(IPEndPoint(ip_address, port), backlog);
}

int TransportServerSocket::Accept(std::unique_ptr<StreamSocket>* socket,
                                  CompletionOnceCallback callback,
                                  IPEndPoint* peer_address) {
  if (peer_address) {
    *peer_address = IPEndPoint();
  }
  return Accept(socket, std::move(callback));
}

}  // namespace crnet
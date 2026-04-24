// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/socket/udp/datagram_server_socket.h"

#include "cr_base/strings/string_piece.h"

#include "cr_net/base/ip_address.h"
#include "cr_net/base/ip_endpoint.h"
#include "cr_net/base/net_errors.h"

namespace crnet {

int DatagramServerSocket::ListenWithAddressAndPort(
    const cr::StringPiece& address_string,
    uint16_t port) {
  IPAddress ip_address;
  if (!ip_address.AssignFromIPLiteral(address_string)) {
    return ERR_ADDRESS_INVALID;
  }

  return Listen(IPEndPoint(ip_address, port));
}

}  // namespace crnet
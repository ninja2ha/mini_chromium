// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_SOCKET_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_SOCKET_H_

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crnet/socket/udp/udp_socket_win.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include "crnet/socket/udp/udp_socket_posix.h"
#endif

namespace crnet {

// UDPSocket
// Accessor API for a UDP socket in either client or server form.
//
// Client form:
// In this case, we're connecting to a specific server, so the client will
// usually use:
//       Open(address_family)  // Open a socket.
//       Connect(address)      // Connect to a UDP server
//       Read/Write            // Reads/Writes all go to a single destination
//
// Server form:
// In this case, we want to read/write to many clients which are connecting
// to this server.  First the server 'binds' to an addres, then we read from
// clients and write responses to them.
// Example:
//       Open(address_family)  // Open a socket.
//       Bind(address/port)    // Binds to port for reading from clients
//       RecvFrom/SendTo       // Each read can come from a different client
//                             // Writes need to be directed to a specific
//                             // address.
#if defined(MINI_CHROMIUM_OS_WIN)
typedef UDPSocketWin UDPSocket;
#elif defined(MINI_CHROMIUM_OS_POSIX)
typedef UDPSocketPosix UDPSocket;
#endif

}  // namespace net

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_SOCKET_H_
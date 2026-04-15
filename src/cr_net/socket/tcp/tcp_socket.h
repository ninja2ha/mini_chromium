// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SOCKET_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SOCKET_H_

#include "cr_net/net_export.h"
#include "cr_net/socket/socket_descriptor.h"
#include "cr_build/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_net/socket/tcp/tcp_socket_win.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include "cr_net/socket/tcp/tcp_socket_posix.h"
#endif

namespace crnet {

// TCPSocket provides a platform-independent interface for TCP sockets.
//
// It is recommended to use TCPClientSocket/TCPServerSocket instead of this
// class, unless a clear separation of client and server socket functionality is
// not suitable for your use case (e.g., a socket needs to be created and bound
// before you know whether it is a client or server socket).
#if defined(MINI_CHROMIUM_OS_WIN)
typedef TCPSocketWin TCPSocket;
#elif defined(MINI_CHROMIUM_OS_POSIX)
typedef TCPSocketPosix TCPSocket;
#endif

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SOCKET_H_
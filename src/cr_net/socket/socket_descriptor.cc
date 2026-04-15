// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/socket/socket_descriptor.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"
#include "cr_net/base/win/winsock_init.h"

#include <ws2tcpip.h>

#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <sys/socket.h>
#include <sys/types.h>
#endif

#if defined(MINI_CHROMIUM_OS_APPLE)
#include <unistd.h>
#endif

namespace crnet {

SocketDescriptor CreatePlatformSocket(int family, int type, int protocol) {
#if defined(MINI_CHROMIUM_OS_WIN)
  EnsureWinsockInit();
  SocketDescriptor result = ::WSASocketW(family, type, protocol, nullptr, 0,
                                         WSA_FLAG_OVERLAPPED);
  if (result != kInvalidSocket && family == AF_INET6) {
    DWORD value = 0;
    if (setsockopt(result, IPPROTO_IPV6, IPV6_V6ONLY,
                   reinterpret_cast<const char*>(&value), sizeof(value))) {
      closesocket(result);
      return kInvalidSocket;
    }
  }
  return result;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  SocketDescriptor result = ::socket(family, type, protocol);
#if defined(MINI_CHROMIUM_OS_APPLE)
  // Disable SIGPIPE on this socket. Although Chromium globally disables
  // SIGPIPE, the net stack may be used in other consumers which do not do
  // this. SO_NOSIGPIPE is a Mac-only API. On Linux, it is a flag on send.
  if (result != kInvalidSocket) {
    int value = 1;
    if (setsockopt(result, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value))) {
      close(result);
      return kInvalidSocket;
    }
  }
#endif
  return result;
#endif  // MINI_CHROMIUM_OS_WIN

}

}  // namespace crnet
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/socket/socket_descriptor.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <ws2tcpip.h>
#include "crnet/base/winsock_init.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <sys/socket.h>
#include <sys/types.h>
#endif

namespace crnet {

SocketDescriptor CreatePlatformSocket(int family, int type, int protocol) {
#if defined(MINI_CHROMIUM_OS_WIN)
  EnsureWinsockInit();
  SocketDescriptor result = ::WSASocket(family, type, protocol, nullptr, 0,
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
  return result;
#endif  // OS_WIN

}

}  // namespace crnet
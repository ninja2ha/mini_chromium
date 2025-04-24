// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_DESCRIPTOR_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_DESCRIPTOR_H_

#include "crbase/build_platform.h"
#include "crnet/base/net_export.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <winsock2.h>
#endif  // OS_WIN

namespace crnet {

#if defined(MINI_CHROMIUM_OS_WIN)
typedef SOCKET SocketDescriptor;
const SocketDescriptor kInvalidSocket = INVALID_SOCKET;
#elif defined(MINI_CHROMIUM_OS_POSIX)
typedef int SocketDescriptor;
const SocketDescriptor kInvalidSocket = -1;
#endif

// Creates  socket. See WSASocket/socket documentation of parameters.
CRNET_EXPORT SocketDescriptor CreatePlatformSocket(int family,
                                                   int type,
                                                   int protocol);

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_DESCRIPTOR_H_
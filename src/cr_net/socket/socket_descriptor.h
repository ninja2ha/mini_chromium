// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_DESCRIPTOR_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_DESCRIPTOR_H_

#include "cr_net/net_export.h"
#include "cr_build/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"
#include <winsock2.h>
#endif  // MINI_CHROMIUM_OS_WIN

namespace crnet {

#if defined(MINI_CHROMIUM_OS_WIN)
typedef SOCKET SocketDescriptor;
const SocketDescriptor kInvalidSocket = INVALID_SOCKET;
#elif defined(MINI_CHROMIUM_OS_POSIX)
typedef int SocketDescriptor;
const SocketDescriptor kInvalidSocket = -1;
#endif

// Creates  socket. See WSASocket/socket documentation of parameters.
SocketDescriptor CRNET_EXPORT CreatePlatformSocket(int family,
                                                   int type,
                                                   int protocol);

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_DESCRIPTOR_H_
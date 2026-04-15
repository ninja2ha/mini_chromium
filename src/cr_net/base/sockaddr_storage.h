// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_SOCKADDR_STORAGE_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_SOCKADDR_STORAGE_H_

#include "cr_net/net_export.h"
#include "cr_build/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <sys/socket.h>
#include <sys/types.h>
#endif

namespace crnet {

// Convenience struct for when you need a |struct sockaddr|.
struct CRNET_EXPORT SockaddrStorage {
  SockaddrStorage();
  SockaddrStorage(const SockaddrStorage& other);
  void operator=(const SockaddrStorage& other);

  struct sockaddr_storage addr_storage;
  socklen_t addr_len;
  struct sockaddr* const addr;
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_SOCKADDR_STORAGE_H_
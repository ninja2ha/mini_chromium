// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_SOCKADDR_STORAGE_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_SOCKADDR_STORAGE_H_

#include "crbase/build_platform.h"
#include "crnet/base/net_export.h"

#if defined(MINI_CHROMIUM_OS_WIN)
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
  SockaddrStorage& operator=(const SockaddrStorage& other);

  const sockaddr* addr() const {
    return reinterpret_cast<const sockaddr*>(&addr_storage);
  }
  sockaddr* addr() { return reinterpret_cast<sockaddr*>(&addr_storage); }

  sockaddr_storage addr_storage;
  socklen_t addr_len = sizeof(addr_storage);
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_SOCKADDR_STORAGE_H_
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/base/sockaddr_storage.h"

#include <string.h>

namespace crnet {

  SockaddrStorage::SockaddrStorage() = default;

SockaddrStorage::SockaddrStorage(const SockaddrStorage& other)
    : addr_len(other.addr_len) {
  memcpy(&addr_storage, &other.addr_storage, sizeof(addr_storage));
}

SockaddrStorage& SockaddrStorage::operator=(const SockaddrStorage& other) {
  addr_len = other.addr_len;
  // addr is already set to &this->addr_storage by default ctor.
  memcpy(&addr_storage, &other.addr_storage, sizeof(addr_storage));
  return *this;
}

}  // namespace net
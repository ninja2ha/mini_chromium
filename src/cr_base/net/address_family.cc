// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/net/address_family.h"

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
typedef struct IUnknown IUnknown;
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "cr_base/logging/logging.h"
#include "cr_base/net/ip_address.h"

namespace cr {
namespace net {

AddressFamily GetAddressFamily(const IPAddress& address) {
  if (address.IsIPv4()) {
    return ADDRESS_FAMILY_IPV4;
  } else if (address.IsIPv6()) {
    return ADDRESS_FAMILY_IPV6;
  } else {
    return ADDRESS_FAMILY_UNSPECIFIED;
  }
}

int ConvertAddressFamily(AddressFamily address_family) {
  switch (address_family) {
    case ADDRESS_FAMILY_UNSPECIFIED:
      return AF_UNSPEC;
    case ADDRESS_FAMILY_IPV4:
      return AF_INET;
    case ADDRESS_FAMILY_IPV6:
      return AF_INET6;
  }
  CR_NOTREACHED();
  return AF_UNSPEC;
}

}  // namespace net
}  // namespace cr

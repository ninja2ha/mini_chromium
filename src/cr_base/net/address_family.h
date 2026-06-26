// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_NET_ADDRESS_FAMILY_H_
#define MINI_CHROMIUM_SRC_CRBASE_NET_ADDRESS_FAMILY_H_

#include "cr_base/base_export.h"

namespace cr {
namespace net {

class IPAddress;

// Enum wrapper around the address family types supported by host resolver
// procedures.
enum AddressFamily {
  ADDRESS_FAMILY_UNSPECIFIED,   // AF_UNSPEC
  ADDRESS_FAMILY_IPV4,          // AF_INET
  ADDRESS_FAMILY_IPV6,          // AF_INET6
  ADDRESS_FAMILY_LAST = ADDRESS_FAMILY_IPV6
};
// Returns AddressFamily for |address|.
CRBASE_EXPORT AddressFamily GetAddressFamily(const IPAddress& address);

// Maps the given AddressFamily to either AF_INET, AF_INET6 or AF_UNSPEC.
CRBASE_EXPORT int ConvertAddressFamily(AddressFamily address_family);

}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_NET_ADDRESS_FAMILY_H_

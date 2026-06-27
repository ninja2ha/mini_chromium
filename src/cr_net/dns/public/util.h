// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_UTIL_H_
#define MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_UTIL_H_

#include <string>

#include "cr_base/strings/string_piece.h"
#include "cr_base/net/address_family.h"
#include "cr_base/net/ip_endpoint.h"

#include "cr_net/base/net_export.h"

namespace cr {
namespace net {

// Basic utility functions for interaction with MDNS and host resolution.
namespace dns_util {

// Returns true if the URI template is acceptable for sending requests. If so,
// the |server_method| is set to "GET" if the template contains a "dns" variable
// and to "POST" otherwise. Any "dns" variable may not be part of the hostname,
// and the expanded template must parse to a valid HTTPS URL.
CRNET_EXPORT bool IsValidDohTemplate(cr::StringPiece server_template,
                                     std::string* server_method);

// Gets the endpoint for the multicast group a socket should join to receive
// MDNS messages. Such sockets should also bind to the endpoint from
// GetMDnsReceiveEndPoint().
//
// This is also the endpoint messages should be sent to to send MDNS messages.
CRNET_EXPORT IPEndPoint GetMdnsGroupEndPoint(AddressFamily address_family);

// Gets the endpoint sockets should be bound to to receive MDNS messages. Such
// sockets should also join the multicast group from GetMDnsGroupEndPoint().
CRNET_EXPORT IPEndPoint GetMdnsReceiveEndPoint(AddressFamily address_family);

}  // namespace dns_util
}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_UTIL_H_
// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_NET_INTERNAL_IP_NUMBER_CONVERSION_H_
#define MINI_CHROMIUM_SRC_CRBASE_NET_INTERNAL_IP_NUMBER_CONVERSION_H_

#include <string>

#include "cr_base/base_export.h"
#include "cr_base/strings/string_piece.h"

namespace cr {
namespace net {

namespace internal {

CRBASE_EXPORT void AppendIPv4Address(const unsigned char address[4], 
                                     std::string* output);
CRBASE_EXPORT void AppendIPv6Address(const unsigned char address[16], 
                                     std::string* output);


// Converts an IPv4 address to a 32-bit number (network byte order).
//
// On success, IPv4 address was successfully parsed. 
// |num_ipv4_components| will be populated with the number of components in the
// IPv4 address.
CRBASE_EXPORT bool IPv4AddressToNumber(StringPiece ip_ip_literal,
                                       unsigned char address[4],
                                       int* num_ipv4_components);

// Converts an IPv6 address to a 128-bit number (network byte order), returning
// true on success. False means that the input was not a valid IPv6 address.
//
// NOTE that |host| is expected to be surrounded by square brackets.
// i.e. "[::1]" rather than "::1".
CRBASE_EXPORT bool IPv6AddressToNumber(StringPiece ip_ip_literal,
                                       unsigned char address[16]);

}  // namesapce internal

}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_NET_INTERNAL_IP_NUMBER_CONVERSION_H_
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/dns/public/util.h"

#include <set>
#include <unordered_map>

#include "cr_base/compiler_config.h"
#include "cr_base/logging/logging.h"
#include "cr_base/net/ip_address.h"
#include "cr_base/strings/string_util.h"

#include "cr_net/dns/public/dns_protocol.h"
#include "cr_net/third_party/uri_template/uri_template.h"

namespace cr {
namespace net {

namespace {
IPEndPoint GetMdnsIPEndPoint(const char* address) {
  IPAddress multicast_group_number;
  bool success = multicast_group_number.AssignFromIPLiteral(address);
  CR_DCHECK(success);
  return IPEndPoint(multicast_group_number,
                    dns_protocol::kDefaultPortMulticast);
}
}  // namespace

namespace dns_util {

bool IsValidDohTemplate(cr::StringPiece server_template,
                        std::string* server_method) {
  std::string url_string;
  std::string test_query = "this_is_a_test_query";
  std::unordered_map<std::string, std::string> template_params(
      {{"dns", test_query}});
  std::set<std::string> vars_found;
  bool valid_template = uri_template::Expand(
      std::string(server_template), template_params, &url_string, &vars_found);
  if (!valid_template) {
    // The URI template is malformed.
    return false;
  }

  // The expanded template must be a valid HTTPS URL.
  if (!cr::StartsWith(url_string, "https://",
                      cr::CompareCase::INSENSITIVE_ASCII)) {
    return false;
  }

  cr::StringPiece piece = cr::StringPiece(url_string);
  piece.remove_prefix(8);
  piece = piece.substr(0, piece.find_first_of('/'));

  if (piece.find(test_query) != cr::StringPiece::npos) {
    // The dns variable may not be part of the hostname.
    return false;
  }
  // If the template contains a dns variable, use GET, otherwise use POST.
  if (server_method) {
    *server_method =
        (vars_found.find("dns") == vars_found.end()) ? "POST" : "GET";
  }
  return true;
}

IPEndPoint GetMdnsGroupEndPoint(AddressFamily address_family) {
  switch (address_family) {
    case ADDRESS_FAMILY_IPV4:
      return GetMdnsIPEndPoint(dns_protocol::kMdnsMulticastGroupIPv4);
    case ADDRESS_FAMILY_IPV6:
      return GetMdnsIPEndPoint(dns_protocol::kMdnsMulticastGroupIPv6);
    default:
      CR_NOTREACHED();
      return IPEndPoint();
  }
}

IPEndPoint GetMdnsReceiveEndPoint(AddressFamily address_family) {
// TODO(qingsi): MacOS should follow other POSIX platforms in the else-branch
// after addressing crbug.com/899310. We have encountered a conflicting issue on
// CrOS as described in crbug.com/931916, and the following is a temporary
// mitigation to reconcile the two issues. Remove this after closing
// crbug.com/899310.
#if defined(MINI_CHROMIUM_OS_WIN)
  // With Windows, binding to a mulitcast group address is not allowed.
  // Multicast messages will be received appropriate to the multicast groups the
  // socket has joined. Sockets intending to receive multicast messages should
  // bind to a wildcard address (e.g. 0.0.0.0).
  switch (address_family) {
    case ADDRESS_FAMILY_IPV4:
      return IPEndPoint(IPAddress::IPv4AllZeros(),
                        dns_protocol::kDefaultPortMulticast);
    case ADDRESS_FAMILY_IPV6:
      return IPEndPoint(IPAddress::IPv6AllZeros(),
                        dns_protocol::kDefaultPortMulticast);
    default:
      CR_NOTREACHED();
      return IPEndPoint();
  }
#else   // !(defined(MINI_CHROMIUM_OS_WIN)
  // With POSIX, any socket can receive messages for multicast groups joined by
  // any socket on the system. Sockets intending to receive messages for a
  // specific multicast group should bind to that group address.
  return GetMdnsGroupEndPoint(address_family);
#endif  // !(defined(MINI_CHROMIUM_OS_WIN)
}

}  // namespace dns_util
}  // namespace net
}  // namespace cr

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_DNS_DNS_HOSTS_H_
#define MINI_CHROMIUM_SRC_CRNET_DNS_DNS_HOSTS_H_

#include <stddef.h>

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cr_base/files/file_path.h"
#include "cr_base/strings/string_piece.h"
#include "cr_base/net/address_family.h"
#include "cr_base/net/ip_address.h"

#include "cr_net/base/net_export.h"

namespace cr {
namespace net {

using DnsHostsKey = std::pair<std::string, AddressFamily>;

struct DnsHostsKeyHash {
  std::size_t operator()(const DnsHostsKey& key) const {
    return StringPieceHasher()(key.first) + key.second;
  }
};

// There are OS-specific variations in how commas in the hosts file behave.
enum ParseHostsCommaMode {
  // Comma is treated as part of a hostname:
  // "127.0.0.1 foo,bar" parses as "foo,bar" mapping to "127.0.0.1".
  PARSE_HOSTS_COMMA_IS_TOKEN,

  // Comma is treated as a hostname separator:
  // "127.0.0.1 foo,bar" parses as "foo" and "bar" both mapping to "127.0.0.1".
  PARSE_HOSTS_COMMA_IS_WHITESPACE,
};

// Parsed results of a Hosts file.
//
// Although Hosts files map IP address to a list of domain names, for name
// resolution the desired mapping direction is: domain name to IP address.
// When parsing Hosts, we apply the "first hit" rule as Windows and glibc do.
// With a Hosts file of:
// 300.300.300.300 localhost # bad ip
// 127.0.0.1 localhost
// 10.0.0.1 localhost
// The expected resolution of localhost is 127.0.0.1.
using DnsHosts = std::unordered_map<DnsHostsKey, IPAddress, DnsHostsKeyHash>;

// Parses |contents| (as read from /etc/hosts or equivalent) and stores results
// in |dns_hosts|. Invalid lines are ignored (as in most implementations).
CRNET_EXPORT void ParseHosts(const std::string& contents,
                             DnsHosts* dns_hosts);

// As above but reads the file pointed to by |path|.
CRNET_EXPORT bool ParseHostsFile(const cr::FilePath& path,
                                 DnsHosts* dns_hosts);



}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRNET_DNS_DNS_HOSTS_H_
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_QUERY_TYPE_H_
#define MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_QUERY_TYPE_H_

#include "cr_base/stl_util.h"
#include "cr_net/base/net_export.h"

namespace cr {
namespace net {

// DNS query type for HostResolver requests.
// See:
// https://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml#dns-parameters-4
enum class DnsQueryType {
  UNSPECIFIED,
  A,
  AAAA,
  TXT,
  PTR,
  SRV,
  INTEGRITY,
  HTTPS,
  MAX = HTTPS
};

const DnsQueryType kDnsQueryTypes[] = {
    DnsQueryType::UNSPECIFIED, DnsQueryType::A,    DnsQueryType::AAAA,
    DnsQueryType::TXT,         DnsQueryType::PTR,  DnsQueryType::SRV,
    DnsQueryType::INTEGRITY,   DnsQueryType::HTTPS};

static_assert(cr::size(kDnsQueryTypes) ==
                  static_cast<unsigned>(DnsQueryType::MAX) + 1,
              "All DnsQueryType values should be in kDnsQueryTypes.");

// |true| iff |dns_query_type| is an address-resulting type, convertable to and
// from net::AddressFamily.
CRNET_EXPORT bool IsAddressType(DnsQueryType dns_query_type);

}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_QUERY_TYPE_H_
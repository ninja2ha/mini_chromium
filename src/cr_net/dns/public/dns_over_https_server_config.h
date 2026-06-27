// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_OVER_HTTPS_SERVER_CONFIG_H_
#define MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_OVER_HTTPS_SERVER_CONFIG_H_

#include <string>

#include "cr_net/base/net_export.h"

namespace cr {
namespace net {

// Simple representation of a DoH server for use in configurations.
struct CRNET_EXPORT DnsOverHttpsServerConfig {
  DnsOverHttpsServerConfig(const std::string& server_template, bool use_post);

  bool operator==(const DnsOverHttpsServerConfig& other) const;

  std::string server_template;
  bool use_post;
};

}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_OVER_HTTPS_SERVER_CONFIG_H_
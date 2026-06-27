// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_CONFIG_OVERRIDES_H_
#define MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_CONFIG_OVERRIDES_H_

#include <string>
#include <vector>

#include "cr_base/containers/optional.h"
#include "cr_base/time/time.h"
#include "cr_base/net/ip_endpoint.h"

#include "cr_net/base/net_export.h"
#include "cr_net/dns/public/dns_over_https_server_config.h"
#include "cr_net/dns/public/secure_dns_mode.h"

namespace cr {
namespace net {

struct DnsConfig;

// Overriding values to be applied over a DnsConfig struct.
struct CRNET_EXPORT DnsConfigOverrides {
  DnsConfigOverrides();
  DnsConfigOverrides(const DnsConfigOverrides& other);
  DnsConfigOverrides(DnsConfigOverrides&& other);
  ~DnsConfigOverrides();

  DnsConfigOverrides& operator=(const DnsConfigOverrides& other);
  DnsConfigOverrides& operator=(DnsConfigOverrides&& other);

  bool operator==(const DnsConfigOverrides& other) const;
  bool operator!=(const DnsConfigOverrides& other) const;

  // Creation method that initializes all values with the defaults from
  // DnsConfig. Guarantees the result of OverridesEverything() will be |true|.
  static DnsConfigOverrides CreateOverridingEverythingWithDefaults();

  // Creates a new DnsConfig where any field with an overriding value in |this|
  // is replaced with that overriding value. Any field without an overriding
  // value (|base::nullopt|) will be copied as-is from |config|.
  DnsConfig ApplyOverrides(const DnsConfig& config) const;

  // Returns |true| if the overriding configuration is comprehensive and would
  // override everything in a base DnsConfig. This is the case if all Optional
  // fields have a value.
  bool OverridesEverything() const;

  // Overriding values. See same-named fields in DnsConfig for explanations.
  cr::Optional<std::vector<IPEndPoint>> nameservers;
  cr::Optional<std::vector<std::string>> search;
  cr::Optional<bool> append_to_multi_label_name;
  cr::Optional<int> ndots;
  cr::Optional<cr::TimeDelta> fallback_period;
  cr::Optional<int> attempts;
  cr::Optional<int> doh_attempts;
  cr::Optional<bool> rotate;
  cr::Optional<bool> use_local_ipv6;
  cr::Optional<std::vector<DnsOverHttpsServerConfig>> dns_over_https_servers;
  cr::Optional<SecureDnsMode> secure_dns_mode;
  cr::Optional<bool> allow_dns_over_https_upgrade;
  cr::Optional<std::vector<std::string>> disabled_upgrade_providers;

  // |hosts| is not supported for overriding except to clear it.
  bool clear_hosts = false;

  // Note no overriding value for |unhandled_options|. It is meta-configuration,
  // and there should be no reason to override it.
};

}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRNET_DNS_PUBLIC_DNS_CONFIG_OVERRIDES_H_
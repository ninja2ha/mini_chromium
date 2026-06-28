// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/dns/dns_util.h"

#include <errno.h>
#include <limits.h>

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "cr_base/compiler_config.h"

#include "cr_base/byte_order.h"
#include "cr_base/data_stream/byte_buffer.h"
#include "cr_base/containers/optional.h"
#include "cr_base/stl_util.h"
#include "cr_base/strings/string_number_conversions.h"
#include "cr_base/strings/string_split.h"

#include "cr_net/base/address_list.h"
#include "cr_net/dns/public/dns_protocol.h"
#include "cr_net/dns/public/doh_provider_entry.h"
#include "cr_net/dns/public/util.h"
#include "cr_net/third_party/uri_template/uri_template.h"
///#include "url/url_canon.h"

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#endif  // defined(MINI_CHROMIUM_OS_POSIX)

namespace cr {
namespace net {
namespace {

// Based on DJB's public domain code.
bool DNSDomainFromDot(const cr::StringPiece& dotted,
                      bool is_unrestricted,
                      std::string* out) {
  const char* buf = dotted.data();
  size_t n = dotted.size();
  char label[dns_protocol::kMaxLabelLength];
  size_t labellen = 0; /* <= sizeof label */
  char name[dns_protocol::kMaxNameLength];
  size_t namelen = 0; /* <= sizeof name */
  char ch;

  for (;;) {
    if (!n)
      break;
    ch = *buf++;
    --n;
    if (ch == '.') {
      // Don't allow empty labels per http://crbug.com/456391.
      if (!labellen)
        return false;
      if (namelen + labellen + 1 > sizeof name)
        return false;
      name[namelen++] = static_cast<char>(labellen);
      memcpy(name + namelen, label, labellen);
      namelen += labellen;
      labellen = 0;
      continue;
    }
    if (labellen >= sizeof label)
      return false;
    if (!is_unrestricted && !IsValidHostLabelCharacter(ch, labellen == 0)) {
      return false;
    }
    label[labellen++] = ch;
  }

  // Allow empty label at end of name to disable suffix search.
  if (labellen) {
    if (namelen + labellen + 1 > sizeof name)
      return false;
    name[namelen++] = static_cast<char>(labellen);
    memcpy(name + namelen, label, labellen);
    namelen += labellen;
    labellen = 0;
  }

  if (namelen + 1 > sizeof name)
    return false;
  if (namelen == 0)  // Empty names e.g. "", "." are not valid.
    return false;
  name[namelen++] = 0;  // This is the root label (of length 0).

  *out = std::string(name, namelen);
  return true;
}

DohProviderEntry::List GetDohProviderEntriesFromNameservers(
    const std::vector<IPEndPoint>& dns_servers,
    const std::vector<std::string>& excluded_providers) {
  const DohProviderEntry::List& providers = DohProviderEntry::GetList();
  DohProviderEntry::List entries;

  for (const auto& server : dns_servers) {
    for (const auto* entry : providers) {
      if (cr::contains(excluded_providers, entry->provider))
        continue;

      // DoH servers should only be added once.
      if (cr::contains(entry->ip_addresses, server.address()) &&
          !cr::contains(entries, entry)) {
        entries.push_back(entry);
      }
    }
  }
  return entries;
}

}  // namespace

bool DNSDomainFromDot(const cr::StringPiece& dotted, std::string* out) {
  return DNSDomainFromDot(dotted, false /* is_unrestricted */, out);
}

bool DNSDomainFromUnrestrictedDot(const cr::StringPiece& dotted,
                                  std::string* out) {
  return DNSDomainFromDot(dotted, true /* is_unrestricted */, out);
}

bool IsValidDNSDomain(const cr::StringPiece& dotted) {
  std::string dns_formatted;
  return DNSDomainFromDot(dotted, &dns_formatted);
}

bool IsValidUnrestrictedDNSDomain(const cr::StringPiece& dotted) {
  std::string dns_formatted;
  return DNSDomainFromUnrestrictedDot(dotted, &dns_formatted);
}

bool IsValidHostLabelCharacter(char c, bool is_first_char) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || (!is_first_char && c == '-') || c == '_';
}

cr::Optional<std::string> DnsDomainToString(cr::StringPiece dns_name,
                                            bool require_complete) {
  cr::NetByteBufferReader reader(dns_name.data(), dns_name.length());
  return DnsDomainToString(reader, require_complete);
}

cr::Optional<std::string> DnsDomainToString(cr::NetByteBufferReader& reader,
                                            bool require_complete) {
  std::string ret;
  size_t octets_read = 0;
  while (reader.Length() > 0) {
    // DNS name compression not allowed because it does not make sense without
    // the context of a full DNS message.
    if ((*reader.Data() & dns_protocol::kLabelMask) ==
        dns_protocol::kLabelPointer)
      return cr::nullopt;

    cr::StringPiece label;
    if (!reader.ReadU8LengthPrefixed(&label))
      return cr::nullopt;
    octets_read += label.size() + 1;

    if (label.size() > dns_protocol::kMaxLabelLength)
      return cr::nullopt;
    if (octets_read > dns_protocol::kMaxNameLength)
      return cr::nullopt;

    if (label.size() == 0)
      return ret;

    if (!ret.empty())
      ret.append(".");

    ret.append(label.data(), label.size());
  }

  if (require_complete)
    return cr::nullopt;

  // If terminating zero-length label was not included in the input, it still
  // counts against the max name length.
  if (octets_read + 1 > dns_protocol::kMaxNameLength)
    return cr::nullopt;

  return ret;
}

std::string GetURLFromTemplateWithoutParameters(const string& server_template) {
  std::string url_string;
  std::unordered_map<string, string> parameters;
  uri_template::Expand(server_template, parameters, &url_string);
  return url_string;
}

///namespace {
///
///bool GetTimeDeltaForConnectionTypeFromFieldTrial(
///    const char* field_trial,
///    NetworkChangeNotifier::ConnectionType type,
///    cr::TimeDelta* out) {
///  std::string group = cr::FieldTrialList::FindFullName(field_trial);
///  if (group.empty())
///    return false;
///  std::vector<cr::StringPiece> group_parts = cr::SplitStringPiece(
///      group, ":", cr::TRIM_WHITESPACE, cr::SPLIT_WANT_ALL);
///  if (type < 0)
///    return false;
///  size_t type_size = static_cast<size_t>(type);
///  if (type_size >= group_parts.size())
///    return false;
///  int64_t ms;
///  if (!cr::StringToInt64(group_parts[type_size], &ms))
///    return false;
///  *out = cr::TimeDelta::FromMilliseconds(ms);
///  return true;
///}
///
///}  // namespace
///
///cr::TimeDelta GetTimeDeltaForConnectionTypeFromFieldTrialOrDefault(
///    const char* field_trial,
///    cr::TimeDelta default_delta,
///    NetworkChangeNotifier::ConnectionType type) {
///  cr::TimeDelta out;
///  if (!GetTimeDeltaForConnectionTypeFromFieldTrial(field_trial, type, &out))
///    out = default_delta;
///  return out;
///}

AddressListDeltaType FindAddressListDeltaType(const AddressList& a,
                                              const AddressList& b) {
  bool pairwise_mismatch = false;
  bool any_match = false;
  bool any_missing = false;
  bool same_size = a.size() == b.size();

  for (size_t i = 0; i < a.size(); ++i) {
    bool this_match = false;
    for (size_t j = 0; j < b.size(); ++j) {
      if (a[i] == b[j]) {
        any_match = true;
        this_match = true;
        // If there is no match before, and the current match, this means
        // DELTA_OVERLAP.
        if (any_missing)
          return DELTA_OVERLAP;
      } else if (i == j) {
        pairwise_mismatch = true;
      }
    }
    if (!this_match) {
      any_missing = true;
      // If any match has occurred before, then there is no need to compare the
      // remaining addresses. This means DELTA_OVERLAP.
      if (any_match)
        return DELTA_OVERLAP;
    }
  }

  if (same_size && !pairwise_mismatch)
    return DELTA_IDENTICAL;
  else if (same_size && !any_missing)
    return DELTA_REORDERED;
  else if (any_match)
    return DELTA_OVERLAP;
  else
    return DELTA_DISJOINT;
}

std::string CreateNamePointer(uint16_t offset) {
  CR_DCHECK((offset & ~dns_protocol::kOffsetMask) == 0);
  char buf[2];
  cr::SetBE16(buf, offset);
  buf[0] |= dns_protocol::kLabelPointer;
  return std::string(buf, sizeof(buf));
}

uint16_t DnsQueryTypeToQtype(DnsQueryType dns_query_type) {
  switch (dns_query_type) {
    case DnsQueryType::UNSPECIFIED:
      CR_NOTREACHED();
      return 0;
    case DnsQueryType::A:
      return dns_protocol::kTypeA;
    case DnsQueryType::AAAA:
      return dns_protocol::kTypeAAAA;
    case DnsQueryType::TXT:
      return dns_protocol::kTypeTXT;
    case DnsQueryType::PTR:
      return dns_protocol::kTypePTR;
    case DnsQueryType::SRV:
      return dns_protocol::kTypeSRV;
    case DnsQueryType::INTEGRITY:
      return dns_protocol::kExperimentalTypeIntegrity;
    case DnsQueryType::HTTPS:
      return dns_protocol::kTypeHttps;
  }

  CR_NOTREACHED();
  return 0;
}

DnsQueryType AddressFamilyToDnsQueryType(AddressFamily address_family) {
  switch (address_family) {
    case ADDRESS_FAMILY_UNSPECIFIED:
      return DnsQueryType::UNSPECIFIED;
    case ADDRESS_FAMILY_IPV4:
      return DnsQueryType::A;
    case ADDRESS_FAMILY_IPV6:
      return DnsQueryType::AAAA;
    default:
      CR_NOTREACHED();
      return DnsQueryType::UNSPECIFIED;
  }
}

std::vector<DnsOverHttpsServerConfig> GetDohUpgradeServersFromDotHostname(
    const std::string& dot_server,
    const std::vector<std::string>& excluded_providers) {
  std::vector<DnsOverHttpsServerConfig> doh_servers;

  if (dot_server.empty())
    return doh_servers;

  for (const auto* entry : DohProviderEntry::GetList()) {
    if (cr::contains(excluded_providers, entry->provider))
      continue;

    if (cr::contains(entry->dns_over_tls_hostnames, dot_server)) {
      std::string server_method;
      CR_CHECK(dns_util::IsValidDohTemplate(entry->dns_over_https_template,
                                            &server_method));
      doh_servers.emplace_back(entry->dns_over_https_template,
                               server_method == "POST");
    }
  }
  return doh_servers;
}

std::vector<DnsOverHttpsServerConfig> GetDohUpgradeServersFromNameservers(
    const std::vector<IPEndPoint>& dns_servers,
    const std::vector<std::string>& excluded_providers) {
  const auto entries =
      GetDohProviderEntriesFromNameservers(dns_servers, excluded_providers);
  std::vector<DnsOverHttpsServerConfig> doh_servers;
  doh_servers.reserve(entries.size());
  std::transform(entries.begin(), entries.end(),
                 std::back_inserter(doh_servers), [](const auto* entry) {
                   std::string server_method;
                   CR_CHECK(dns_util::IsValidDohTemplate(
                       entry->dns_over_https_template, &server_method));
                   return DnsOverHttpsServerConfig(
                       entry->dns_over_https_template, server_method == "POST");
                 });
  return doh_servers;
}

std::string GetDohProviderIdForHistogramFromDohConfig(
    const DnsOverHttpsServerConfig& doh_server) {
  const auto& entries = DohProviderEntry::GetList();
  const auto it =
      std::find_if(entries.begin(), entries.end(), [&](const auto* entry) {
        return entry->dns_over_https_template == doh_server.server_template;
      });
  return it != entries.end() ? (*it)->provider : "Other";
}

std::string GetDohProviderIdForHistogramFromNameserver(
    const IPEndPoint& nameserver) {
  const auto entries = GetDohProviderEntriesFromNameservers({nameserver}, {});
  return entries.empty() ? "Other" : entries[0]->provider;
}

std::string SecureDnsModeToString(const SecureDnsMode secure_dns_mode) {
  switch (secure_dns_mode) {
    case SecureDnsMode::kOff:
      return "Off";
    case SecureDnsMode::kAutomatic:
      return "Automatic";
    case SecureDnsMode::kSecure:
      return "Secure";
  }

  CR_NOTREACHED();
  return "";
}

}  // namespace net
}  // namespace cr

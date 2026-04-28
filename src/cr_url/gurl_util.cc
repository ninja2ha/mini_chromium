// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_url/gurl_util.h"

#include "cr_build/build_config.h"

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <netinet/in.h>
#elif defined(MINI_CHROMIUM_OS_WIN)
#endif

#include "cr_base/logging/logging.h"
#include "cr_base/containers/stack_container.h"
#include "cr_base/strings/string_piece.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/strings/stringprintf.h"
#include "cr_base/strings/strcat.h"
#include "cr_base/strings/utf_string_conversions.h"
#include "cr_base/strings/escape.h" 
#include "cr_net/base/ip_address.h"
#include "cr_url/gurl.h"
#include "cr_url/url_canon.h"
#include "cr_url/url_canon_ip.h"
#include "cr_url/url_constants.h"
#include "cr_url/url_util.h"

namespace crurl {

namespace {

bool IsHostCharAlphanumeric(char c) {
  // We can just check lowercase because uppercase characters have already been
  // normalized.
  return ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9'));
}

bool IsNormalizedLocalhostTLD(const std::string& host) {
  return cr::EndsWith(host, ".localhost");
}

// Helper function used by GetIdentityFromURL. If |escaped_text| can be "safely
// unescaped" to a valid UTF-8 string, return that string, as UTF-16. Otherwise,
// convert it as-is to UTF-16. "Safely unescaped" is defined as having no
// escaped character between '0x00' and '0x1F', inclusive.
std::u16string UnescapeIdentityString(cr::StringPiece escaped_text) {
  std::string unescaped_text;
  if (cr::UnescapeBinaryURLComponentSafe(
          escaped_text, false /* fail_on_path_separators */, &unescaped_text)) {
    std::u16string result;
    if (cr::UTF8ToUTF16(unescaped_text.data(), unescaped_text.length(),
                        &result)) {
      return result;
    }
  }
  return cr::UTF8ToUTF16(escaped_text);
}

// copy from cr_net ip_address.cc
bool ParseIPLiteralToBytes(const cr::StringPiece& ip_literal,
                           cr::StackVector<uint8_t, 16>& bytes) {
  // |ip_literal| could be either an IPv4 or an IPv6 literal. If it contains
  // a colon however, it must be an IPv6 address.
  if (ip_literal.find(':') != cr::StringPiece::npos) {
    // GURL expects IPv6 hostnames to be surrounded with brackets.
    std::string host_brackets = cr::StrCat({"[", ip_literal, "]"});

    CR_DCHECK(host_brackets.length() <= 39);
    crurl::Component host_comp(0, static_cast<int>(host_brackets.size()));

    // Try parsing the hostname as an IPv6 literal.
    bytes->resize(16);  // 128 bits.
    return crurl::IPv6AddressToNumber(host_brackets.data(), host_comp,
                                      bytes->data());
  }

  CR_DCHECK(ip_literal.length() <= 15);
  // Otherwise the string is an IPv4 address.
  bytes->resize(4);  // 32 bits.
  crurl::Component host_comp(0, static_cast<int>(ip_literal.size()));
  int num_components;
  crurl::CanonHostInfo::Family family = crurl::IPv4AddressToNumber(
      ip_literal.data(), host_comp, bytes->data(), &num_components);
  return family == crurl::CanonHostInfo::IPV4;
}

}  // namespace

GURL AppendQueryParameter(const GURL& url,
                          const std::string& name,
                          const std::string& value) {
  std::string query(url.query());

  if (!query.empty())
    query += "&";

  query += (cr::EscapeQueryParamValue(name, true) + "=" +
            cr::EscapeQueryParamValue(value, true));
  GURL::Replacements replacements;
  replacements.SetQueryStr(query);
  return url.ReplaceComponents(replacements);
}

GURL AppendOrReplaceQueryParameter(const GURL& url,
                                   const std::string& name,
                                   const std::string& value) {
  bool replaced = false;
  std::string param_name = cr::EscapeQueryParamValue(name, true);
  std::string param_value = cr::EscapeQueryParamValue(value, true);

  const std::string input = url.query();
  // unsafe_cast
  crurl::Component cursor(0, static_cast<int>(input.size()));
  std::string output;
  crurl::Component key_range, value_range;
  while (crurl::ExtractQueryKeyValue(input.data(), &cursor, &key_range,
                                     &value_range)) {
    const cr::StringPiece key(
        input.data() + key_range.begin, key_range.len);
    std::string key_value_pair;
    // Check |replaced| as only the first pair should be replaced.
    if (!replaced && key == param_name) {
      replaced = true;
      key_value_pair = (param_name + "=" + param_value);
    } else {
      key_value_pair.assign(input, key_range.begin,
                            value_range.end() - key_range.begin);
    }
    if (!output.empty())
      output += "&";

    output += key_value_pair;
  }
  if (!replaced) {
    if (!output.empty())
      output += "&";

    output += (param_name + "=" + param_value);
  }
  GURL::Replacements replacements;
  replacements.SetQueryStr(output);
  return url.ReplaceComponents(replacements);
}

QueryIterator::QueryIterator(const GURL& url)
    : url_(url),
      at_end_(!url.is_valid()) {
  if (!at_end_) {
    query_ = url.parsed_for_possibly_invalid_spec().query;
    Advance();
  }
}

QueryIterator::~QueryIterator() = default;

std::string QueryIterator::GetKey() const {
  CR_DCHECK(!at_end_);
  if (key_.is_nonempty())
    return url_.spec().substr(key_.begin, key_.len);
  return std::string();
}

std::string QueryIterator::GetValue() const {
  CR_DCHECK(!at_end_);
  if (value_.is_nonempty())
    return url_.spec().substr(value_.begin, value_.len);
  return std::string();
}

const std::string& QueryIterator::GetUnescapedValue() {
  CR_DCHECK(!at_end_);
  if (value_.is_nonempty() && unescaped_value_.empty()) {
    unescaped_value_ = cr::UnescapeURLComponent(
        GetValue(), 
        cr::UnescapeRule::SPACES | 
          cr::UnescapeRule::PATH_SEPARATORS |
          cr::UnescapeRule::URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS |
          cr::UnescapeRule::REPLACE_PLUS_WITH_SPACE);
  }
  return unescaped_value_;
}

bool QueryIterator::IsAtEnd() const {
  return at_end_;
}

void QueryIterator::Advance() {
  CR_DCHECK (!at_end_);
  key_.reset();
  value_.reset();
  unescaped_value_.clear();
  at_end_ =
      !crurl::ExtractQueryKeyValue(
          url_.spec().c_str(), &query_, &key_, &value_);
}

bool GetValueForKeyInQuery(const GURL& url,
                           const std::string& search_key,
                           std::string* out_value) {
  for (QueryIterator it(url); !it.IsAtEnd(); it.Advance()) {
    if (it.GetKey() == search_key) {
      *out_value = it.GetUnescapedValue();
      return true;
    }
  }
  return false;
}

bool ParseHostAndPort(cr::StringPiece input, std::string* host, int* port) {
  if (input.empty())
    return false;

  // unsafe_cast
  crurl::Component auth_component(0, static_cast<int>(input.size()));
  crurl::Component username_component;
  crurl::Component password_component;
  crurl::Component hostname_component;
  crurl::Component port_component;

  crurl::ParseAuthority(input.data(), auth_component, &username_component,
                        &password_component, &hostname_component,
                        &port_component);

  // There shouldn't be a username/password.
  if (username_component.is_valid() || password_component.is_valid())
    return false;

  if (!hostname_component.is_nonempty())
    return false;  // Failed parsing.

  int parsed_port_number = -1;
  if (port_component.is_nonempty()) {
    parsed_port_number = crurl::ParsePort(input.data(), port_component);

    // If parsing failed, port_number will be either PORT_INVALID or
    // PORT_UNSPECIFIED, both of which are negative.
    if (parsed_port_number < 0)
      return false;  // Failed parsing the port number.
  }

  if (port_component.len == 0)
    return false;  // Reject inputs like "foo:"

  unsigned char tmp_ipv6_addr[16];

  // If the hostname starts with a bracket, it is either an IPv6 literal or
  // invalid. If it is an IPv6 literal then strip the brackets.
  if (hostname_component.len > 0 && input[hostname_component.begin] == '[') {
    if (input[hostname_component.end() - 1] == ']' &&
        crurl::IPv6AddressToNumber(input.data(), hostname_component,
                                   tmp_ipv6_addr)) {
      // Strip the brackets.
      hostname_component.begin++;
      hostname_component.len -= 2;
    } else {
      return false;
    }
  }

  // Pass results back to caller.
  host->assign(input.data() + hostname_component.begin, hostname_component.len);
  *port = parsed_port_number;

  return true;  // Success.
}


std::string GetHostAndPort(const GURL& url) {
  // For IPv6 literals, GURL::host() already includes the brackets so it is
  // safe to just append a colon.
  return cr::StringPrintf("%s:%d", url.host().c_str(),
                          url.EffectiveIntPort());
}

std::string GetHostAndOptionalPort(const GURL& url) {
  // For IPv6 literals, GURL::host() already includes the brackets
  // so it is safe to just append a colon.
  if (url.has_port())
    return cr::StringPrintf("%s:%s", url.host().c_str(), url.port().c_str());
  return url.host();
}

std::string TrimEndingDot(cr::StringPiece host) {
  cr::StringPiece host_trimmed = host;
  size_t len = host_trimmed.length();
  if (len > 1 && host_trimmed[len - 1] == '.') {
    host_trimmed.remove_suffix(1);
  }
  return std::string(host_trimmed);
}

std::string GetHostOrSpecFromURL(const GURL& url) {
  return url.has_host() ? TrimEndingDot(url.host_piece()) : url.spec();
}

std::string GetSuperdomain(cr::StringPiece domain) {
  size_t dot_pos = domain.find('.');
  if (dot_pos == std::string::npos)
    return "";
  return std::string(domain.substr(dot_pos + 1));
}

bool IsSubdomainOf(cr::StringPiece subdomain, cr::StringPiece superdomain) {
  // Subdomain must be identical or have strictly more labels than the
  // superdomain.
  if (subdomain.length() <= superdomain.length())
    return subdomain == superdomain;

  // Superdomain must be suffix of subdomain, and the last character not
  // included in the matching substring must be a dot.
  if (!cr::EndsWith(subdomain, superdomain))
    return false;
  subdomain.remove_suffix(superdomain.length());
  return subdomain.back() == '.';
}

std::string CanonicalizeHost(cr::StringPiece host,
                             crurl::CanonHostInfo* host_info) {
  // Try to canonicalize the host.
  const crurl::Component raw_host_component(0, static_cast<int>(host.length()));
  std::string canon_host;
  crurl::StdStringCanonOutput canon_host_output(&canon_host);
  crurl::CanonicalizeHostVerbose(host.data(), raw_host_component,
                                 &canon_host_output, host_info);

  if (host_info->out_host.is_nonempty() &&
      host_info->family != crurl::CanonHostInfo::BROKEN) {
    // Success!  Assert that there's no extra garbage.
    canon_host_output.Complete();
    CR_DCHECK(host_info->out_host.len == static_cast<int>(canon_host.length()));
  } else {
    // Empty host, or canonicalization failed.  We'll return empty.
    canon_host.clear();
  }

  return canon_host;
}

bool IsCanonicalizedHostCompliant(const std::string& host) {
  if (host.empty())
    return false;

  bool in_component = false;
  bool most_recent_component_started_alphanumeric = false;

  for (std::string::const_iterator i(host.begin()); i != host.end(); ++i) {
    const char c = *i;
    if (!in_component) {
      most_recent_component_started_alphanumeric = IsHostCharAlphanumeric(c);
      if (!most_recent_component_started_alphanumeric && (c != '-') &&
          (c != '_')) {
        return false;
      }
      in_component = true;
    } else if (c == '.') {
      in_component = false;
    } else if (!IsHostCharAlphanumeric(c) && (c != '-') && (c != '_')) {
      return false;
    }
  }

  return most_recent_component_started_alphanumeric;
}

bool IsLocalhost(const GURL& url) {
  return HostStringIsLocalhost(url.HostNoBracketsPiece());
}

bool HostStringIsLocalhost(cr::StringPiece host) {
  cr::StackVector<uint8_t, 16> ip_bytes;
  if (!ParseIPLiteralToBytes(host, ip_bytes)) {
    return IsLocalHostname(host);
  }

  if (ip_bytes->size() == 4) { // ipv4
    return ip_bytes->front() == 127;
  }

  if (ip_bytes->size() == 16) { // ipv6
    for (size_t i = 0; i + 1 < ip_bytes->size(); ++i) {
      if (ip_bytes[i] != 0)
        return false;
    }
    return ip_bytes->back() == 1;
  }

  return false;
}

GURL SimplifyUrlForRequest(const GURL& url) {
  CR_DCHECK(url.is_valid());
  // Fast path to avoid re-canonicalization via ReplaceComponents.
  if (!url.has_username() && !url.has_password() && !url.has_ref())
    return url;
  GURL::Replacements replacements;
  replacements.ClearUsername();
  replacements.ClearPassword();
  replacements.ClearRef();
  return url.ReplaceComponents(replacements);
}

GURL ChangeWebSocketSchemeToHttpScheme(const GURL& url) {
  CR_DCHECK(url.SchemeIsWSOrWSS());
  GURL::Replacements replace_scheme;
  replace_scheme.SetSchemeStr(url.SchemeIs(crurl::kWssScheme) 
      ? crurl::kHttpsScheme : crurl::kHttpScheme);
  return url.ReplaceComponents(replace_scheme);
}

bool IsStandardSchemeWithNetworkHost(cr::StringPiece scheme) {
  // file scheme is special. Windows file share origins can have network hosts.
  if (scheme == crurl::kFileScheme)
    return true;

  crurl::SchemeType scheme_type;
  // unsafe_cast
  if (!crurl::GetStandardSchemeType(
          scheme.data(), crurl::Component(0, static_cast<int>(scheme.length())),
                                          &scheme_type)) {
    return false;
  }
  return scheme_type == crurl::SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION ||
         scheme_type == crurl::SCHEME_WITH_HOST_AND_PORT;
}

void GetIdentityFromURL(const GURL& url,
                        std::u16string* username,
                        std::u16string* password) {
  *username = UnescapeIdentityString(url.username());
  *password = UnescapeIdentityString(url.password());
}

bool IsLocalHostname(cr::StringPiece host) {
  std::string normalized_host = cr::ToLowerASCII(host);
  // Remove any trailing '.'.
  if (!normalized_host.empty() && *normalized_host.rbegin() == '.')
    normalized_host.resize(normalized_host.size() - 1);

  return normalized_host == "localhost" ||
         IsNormalizedLocalhostTLD(normalized_host);
}

}  // namespace crurl
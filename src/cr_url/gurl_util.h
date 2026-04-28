// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains a set of utility functions related to parsing,
// manipulating, and interacting with URLs and hostnames. These functions are
// intended to be of a text-processing nature, and should not attempt to use any
// networking or blocking services.

#ifndef MINI_CHROMIUM_SRC_CRURL_GURL_UTIL_H_
#define MINI_CHROMIUM_SRC_CRURL_GURL_UTIL_H_

#include <string>

#include "cr_base/strings/string_piece.h"
#include "cr_net/net_export.h"
#include "cr_url/third_party/mozilla/url_parse.h"

namespace crurl {
class GURL;
struct CanonHostInfo;
}  // namespace crurl

namespace crurl {

// Returns a new GURL by appending the given query parameter name and the
// value. Unsafe characters in the name and the value are escaped like
// %XX%XX. The original query component is preserved if it's present.
//
// Examples:
//
// AppendQueryParameter(GURL("http://example.com"), "name", "value").spec()
// => "http://example.com?name=value"
// AppendQueryParameter(GURL("http://example.com?x=y"), "name", "value").spec()
// => "http://example.com?x=y&name=value"
  CRURL_EXPORT GURL AppendQueryParameter(const GURL& url,
                                         const std::string& name,
                                         const std::string& value);

// Returns a new GURL by appending or replacing the given query parameter name
// and the value. If |name| appears more than once, only the first name-value
// pair is replaced. Unsafe characters in the name and the value are escaped
// like %XX%XX. The original query component is preserved if it's present.
//
// Examples:
//
// AppendOrReplaceQueryParameter(
//     GURL("http://example.com"), "name", "new").spec()
// => "http://example.com?name=value"
// AppendOrReplaceQueryParameter(
//     GURL("http://example.com?x=y&name=old"), "name", "new").spec()
// => "http://example.com?x=y&name=new"
CRURL_EXPORT GURL AppendOrReplaceQueryParameter(const GURL& url,
                                                const std::string& name,
                                                const std::string& value);

// Iterates over the key-value pairs in the query portion of |url|.
class CRURL_EXPORT QueryIterator {
 public:
  explicit QueryIterator(const GURL& url);
  QueryIterator(const QueryIterator&) = delete;
  QueryIterator& operator=(const QueryIterator&) = delete;
  ~QueryIterator();

  std::string GetKey() const;
  std::string GetValue() const;
  const std::string& GetUnescapedValue();

  bool IsAtEnd() const;
  void Advance();

 private:
  const GURL& url_;
  crurl::Component query_;
  bool at_end_;
  crurl::Component key_;
  crurl::Component value_;
  std::string unescaped_value_;
};

// Looks for |search_key| in the query portion of |url|. Returns true if the
// key is found and sets |out_value| to the unescaped value for the key.
// Returns false if the key is not found.
CRURL_EXPORT bool GetValueForKeyInQuery(const GURL& url,
                                        const std::string& search_key,
                                        std::string* out_value);

// Splits an input of the form <host>[":"<port>] into its consitituent parts.
// Saves the result into |*host| and |*port|. If the input did not have
// the optional port, sets |*port| to -1.
// Returns true if the parsing was successful, false otherwise.
// The returned host is NOT canonicalized, and may be invalid.
//
// IPv6 literals must be specified in a bracketed form, for instance:
//   [::1]:90 and [::1]
//
// The resultant |*host| in both cases will be "::1" (not bracketed).
CRURL_EXPORT bool ParseHostAndPort(cr::StringPiece input,
                                   std::string* host,
                                   int* port);

// Returns a host:port string for the given URL.
CRURL_EXPORT std::string GetHostAndPort(const GURL& url);

// Returns a host[:port] string for the given URL, where the port is omitted
// if it is the default for the URL's scheme.
CRURL_EXPORT std::string GetHostAndOptionalPort(const GURL& url);

// Returns the hostname by trimming the ending dot, if one exists.
CRURL_EXPORT std::string TrimEndingDot(cr::StringPiece host);

// Returns either the host from |url|, or, if the host is empty, the full spec.
CRURL_EXPORT std::string GetHostOrSpecFromURL(const GURL& url);

// Returns the given domain minus its leftmost label, or the empty string if the
// given domain is just a single label. For normal domain names (not IP
// addresses), this represents the "superdomain" of the given domain.
// Note that this does not take into account anything like the Public Suffix
// List, so the superdomain may end up being a bare eTLD. The returned string is
// not guaranteed to be a valid or canonical hostname, or to make any sense at
// all.
//
// Examples:
//
// GetSuperdomain("assets.example.com") -> "example.com"
// GetSuperdomain("example.net") -> "net"
// GetSuperdomain("littlebox") -> ""
// GetSuperdomain("127.0.0.1") -> "0.0.1"
CRURL_EXPORT std::string GetSuperdomain(cr::StringPiece domain);

// Returns whether |subdomain| is a subdomain of (or identical to)
// |superdomain|, if both are hostnames (not IP addresses -- for which this
// function is nonsensical). Does not consider the Public Suffix List.
// Returns true if both input strings are empty.
CRURL_EXPORT bool IsSubdomainOf(cr::StringPiece subdomain,
                                cr::StringPiece superdomain);

// Canonicalizes |host| and returns it.  Also fills |host_info| with
// IP address information.  |host_info| must not be NULL.
CRURL_EXPORT std::string CanonicalizeHost(cr::StringPiece host,
                                          crurl::CanonHostInfo* host_info);

// Returns true if |host| is not an IP address and is compliant with a set of
// rules based on RFC 1738 and tweaked to be compatible with the real world.
// The rules are:
//   * One or more components separated by '.'
//   * Each component contains only alphanumeric characters and '-' or '_'
//   * The last component begins with an alphanumeric character
//   * Optional trailing dot after last component (means "treat as FQDN")
//
// NOTE: You should only pass in hosts that have been returned from
// CanonicalizeHost(), or you may not get accurate results.
CRURL_EXPORT bool IsCanonicalizedHostCompliant(const std::string& host);

// Returns true if the host part of |url| is a local host name according to
// HostStringIsLocalhost.
CRURL_EXPORT bool IsLocalhost(const GURL& url);

// Returns true if |host| is one of the local hostnames
// (e.g. "localhost") or IP addresses (IPv4 127.0.0.0/8 or IPv6 ::1).
// "[::1]" is not detected as a local hostname. Do not use this method to check
// whether the host part of a URL is a local host name; use IsLocalhost instead.
//
// Note that this function does not check for IP addresses other than
// the above, although other IP addresses may point to the local
// machine.
CRURL_EXPORT bool HostStringIsLocalhost(cr::StringPiece host);

// Strip the portions of |url| that aren't core to the network request.
//   - user name / password
//   - reference section
CRURL_EXPORT GURL SimplifyUrlForRequest(const GURL& url);

// Changes scheme "ws" to "http" and "wss" to "https". This is useful for origin
// checks and authentication, where WebSocket URLs are treated as if they were
// HTTP. It is an error to call this function with a url with a scheme other
// than "ws" or "wss".
CRURL_EXPORT GURL ChangeWebSocketSchemeToHttpScheme(const GURL& url);

// Returns whether the given url scheme is of a standard scheme type that can
// have hostnames representing domains (i.e. network hosts).
// See url::SchemeType.
CRURL_EXPORT bool IsStandardSchemeWithNetworkHost(cr::StringPiece scheme);

// Extracts the unescaped username/password from |url|, saving the results
// into |*username| and |*password|.
CRURL_EXPORT void GetIdentityFromURL(const GURL& url,
                                     std::u16string* username,
                                     std::u16string* password);

// This function tests |host| to see if it is of any local hostname form.
// |host| is normalized before being tested.
CRURL_EXPORT bool IsLocalHostname(cr::StringPiece host);

}  // namespace crurl

#endif  // MINI_CHROMIUM_SRC_CRURL_GURL_UTIL_H_
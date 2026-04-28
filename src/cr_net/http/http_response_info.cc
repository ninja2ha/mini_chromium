// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/http/http_response_info.h"

#include "cr_base/logging/logging.h"
#include "cr_base/numerics/safe_conversions.h"
#include "cr_base/time/time.h"
#include "cr_net/base/net_errors.h"
///#include "cr_net/cert/sct_status_flags.h"
///#include "cr_net/cert/signed_certificate_timestamp.h"
///#include "cr_net/cert/x509_certificate.h"
#include "cr_net/http/http_response_headers.h"
///#include "cr_net/ssl/ssl_cert_request_info.h"
///#include "cr_net/ssl/ssl_connection_status_flags.h"
///#include "cr_net/third_party/quiche/src/quic/core/quic_versions.h"
///#include "cr_third_party/boringssl/src/include/openssl/ssl.h"

using cr::Time;

namespace crnet {

// These values can be bit-wise combined to form the flags field of the
// serialized HttpResponseInfo.
enum {
  // The version of the response info used when persisting response info.
  RESPONSE_INFO_VERSION = 3,

  // The minimum version supported for deserializing response info.
  RESPONSE_INFO_MINIMUM_VERSION = 3,

  // We reserve up to 8 bits for the version number.
  RESPONSE_INFO_VERSION_MASK = 0xFF,

  // This bit is set if the response info has a cert at the end.
  // Version 1 serialized only the end-entity certificate, while subsequent
  // versions include the available certificate chain.
  RESPONSE_INFO_HAS_CERT = 1 << 8,

  // This bit was historically set if the response info had a security-bits
  // field (security strength, in bits, of the SSL connection) at the end.
  RESPONSE_INFO_HAS_SECURITY_BITS = 1 << 9,

  // This bit is set if the response info has a cert status at the end.
  RESPONSE_INFO_HAS_CERT_STATUS = 1 << 10,

  // This bit is set if the response info has vary header data.
  RESPONSE_INFO_HAS_VARY_DATA = 1 << 11,

  // This bit is set if the request was cancelled before completion.
  RESPONSE_INFO_TRUNCATED = 1 << 12,

  // This bit is set if the response was received via SPDY.
  RESPONSE_INFO_WAS_SPDY = 1 << 13,

  // This bit is set if the request has ALPN negotiated.
  RESPONSE_INFO_WAS_ALPN = 1 << 14,

  // This bit is set if the request was fetched via an explicit proxy.
  RESPONSE_INFO_WAS_PROXY = 1 << 15,

  // This bit is set if the response info has an SSL connection status field.
  // This contains the ciphersuite used to fetch the resource as well as the
  // protocol version, compression method and whether SSLv3 fallback was used.
  RESPONSE_INFO_HAS_SSL_CONNECTION_STATUS = 1 << 16,

  // This bit is set if the response info has protocol version.
  RESPONSE_INFO_HAS_ALPN_NEGOTIATED_PROTOCOL = 1 << 17,

  // This bit is set if the response info has connection info.
  RESPONSE_INFO_HAS_CONNECTION_INFO = 1 << 18,

  // This bit is set if the request has http authentication.
  RESPONSE_INFO_USE_HTTP_AUTHENTICATION = 1 << 19,

  // This bit is set if ssl_info has SCTs.
  RESPONSE_INFO_HAS_SIGNED_CERTIFICATE_TIMESTAMPS = 1 << 20,

  RESPONSE_INFO_UNUSED_SINCE_PREFETCH = 1 << 21,

  // This bit is set if the response has a key exchange group.
  RESPONSE_INFO_HAS_KEY_EXCHANGE_GROUP = 1 << 22,

  // This bit is set if ssl_info recorded that PKP was bypassed due to a local
  // trust anchor.
  RESPONSE_INFO_PKP_BYPASSED = 1 << 23,

  // This bit is set if stale_revalidate_time is stored.
  RESPONSE_INFO_HAS_STALENESS = 1 << 24,

  // This bit is set if the response has a peer signature algorithm.
  RESPONSE_INFO_HAS_PEER_SIGNATURE_ALGORITHM = 1 << 25,

  // This bit is set if the response is a prefetch whose reuse should be
  // restricted in some way.
  RESPONSE_INFO_RESTRICTED_PREFETCH = 1 << 26,

  // This bit is set if the response has a nonempty `dns_aliases` entry.
  RESPONSE_INFO_HAS_DNS_ALIASES = 1 << 27,

  // TODO(darin): Add other bits to indicate alternate request methods.
  // For now, we don't support storing those.
};

HttpResponseInfo::ConnectionInfoCoarse HttpResponseInfo::ConnectionInfoToCoarse(
    ConnectionInfo info) {
  switch (info) {
    case CONNECTION_INFO_HTTP0_9:
    case CONNECTION_INFO_HTTP1_0:
    case CONNECTION_INFO_HTTP1_1:
      return CONNECTION_INFO_COARSE_HTTP1;

    case CONNECTION_INFO_UNKNOWN:
      return CONNECTION_INFO_COARSE_OTHER;

    case NUM_OF_CONNECTION_INFOS:
      CR_NOTREACHED();
      return CONNECTION_INFO_COARSE_OTHER;
  }

  CR_NOTREACHED();
  return CONNECTION_INFO_COARSE_OTHER;
}

HttpResponseInfo::HttpResponseInfo()
    : was_cached(false),
      cache_entry_status(CacheEntryStatus::ENTRY_UNDEFINED),
      server_data_unavailable(false),
      network_accessed(false),
      was_alpn_negotiated(false),
      was_fetched_via_proxy(false),
      did_use_http_auth(false),
      unused_since_prefetch(false),
      restricted_prefetch(false),
      async_revalidation_requested(false),
      connection_info(CONNECTION_INFO_UNKNOWN) {}

HttpResponseInfo::HttpResponseInfo(const HttpResponseInfo& rhs) = default;

HttpResponseInfo::~HttpResponseInfo() = default;

HttpResponseInfo& HttpResponseInfo::operator=(const HttpResponseInfo& rhs) =
    default;
 
// static
std::string HttpResponseInfo::ConnectionInfoToString(
    ConnectionInfo connection_info) {
  switch (connection_info) {
    case CONNECTION_INFO_UNKNOWN:
      return "unknown";
    case CONNECTION_INFO_HTTP1_1:
      return "http/1.1";
    case CONNECTION_INFO_HTTP0_9:
      return "http/0.9";
    case CONNECTION_INFO_HTTP1_0:
      return "http/1.0";
    case NUM_OF_CONNECTION_INFOS:
      break;
  }
  CR_NOTREACHED();
  return "";
}

}  // namespace crnet
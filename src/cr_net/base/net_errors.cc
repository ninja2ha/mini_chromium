// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/base/net_errors.h"

#include "cr_base/logging/logging.h"
///#include "net/third_party/quiche/src/quic/core/quic_error_codes.h"

namespace crnet {

std::string ErrorToString(int error) {
  return "net::" + ErrorToShortString(error);
}

std::string ErrorToShortString(int error) {
  if (error == OK)
    return "OK";

  const char* error_string;
  switch (error) {
#define NET_ERROR(label, value) \
  case ERR_ ## label: \
    error_string = # label; \
    break;
#include "cr_net/base/internal/net_error_list.h"
#undef NET_ERROR
  default:
    CR_NOTREACHED();
    error_string = "<unknown>";
  }
  return std::string("ERR_") + error_string;
}

bool IsCertificateError(int error) {
  // Certificate errors are negative integers from net::ERR_CERT_BEGIN
  // (inclusive) to net::ERR_CERT_END (exclusive) in *decreasing* order.
  // ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN is currently an exception to this
  // rule.
  return (error <= ERR_CERT_BEGIN && error > ERR_CERT_END) ||
         (error == ERR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN);
}

bool IsClientCertificateError(int error) {
  switch (error) {
    case ERR_BAD_SSL_CLIENT_AUTH_CERT:
    case ERR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED:
    case ERR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY:
    case ERR_SSL_CLIENT_AUTH_SIGNATURE_FAILED:
    case ERR_SSL_CLIENT_AUTH_NO_COMMON_ALGORITHMS:
      return true;
    default:
      return false;
  }
}

bool IsHostnameResolutionError(int error) {
  CR_DCHECK(ERR_NAME_RESOLUTION_FAILED != error);
  return error == ERR_NAME_NOT_RESOLVED;
}

bool IsRequestBlockedError(int error) {
  switch (error) {
    case ERR_BLOCKED_BY_CLIENT:
    case ERR_BLOCKED_BY_ADMINISTRATOR:
    case ERR_BLOCKED_BY_CSP:
      return true;
    default:
      return false;
  }
}

Error FileErrorToNetError(cr::File::Error file_error) {
  switch (file_error) {
    case cr::File::FILE_OK:
      return OK;
    case cr::File::FILE_ERROR_EXISTS:
      return ERR_FILE_EXISTS;
    case cr::File::FILE_ERROR_NOT_FOUND:
      return ERR_FILE_NOT_FOUND;
    case cr::File::FILE_ERROR_ACCESS_DENIED:
      return ERR_ACCESS_DENIED;
    case cr::File::FILE_ERROR_NO_MEMORY:
      return ERR_OUT_OF_MEMORY;
    case cr::File::FILE_ERROR_NO_SPACE:
      return ERR_FILE_NO_SPACE;
    case cr::File::FILE_ERROR_INVALID_OPERATION:
      return ERR_INVALID_ARGUMENT;
    case cr::File::FILE_ERROR_ABORT:
      return ERR_ABORTED;
    case cr::File::FILE_ERROR_INVALID_URL:
      return ERR_INVALID_URL;
    case cr::File::FILE_ERROR_TOO_MANY_OPENED:
      return ERR_INSUFFICIENT_RESOURCES;
    case cr::File::FILE_ERROR_SECURITY:
      return ERR_ACCESS_DENIED;
    case cr::File::FILE_ERROR_MAX:
      CR_NOTREACHED();
      CR_FALLTHROUGH;
    case cr::File::FILE_ERROR_NOT_A_DIRECTORY:
    case cr::File::FILE_ERROR_NOT_A_FILE:
    case cr::File::FILE_ERROR_NOT_EMPTY:
    case cr::File::FILE_ERROR_IO:
    case cr::File::FILE_ERROR_IN_USE:
    // No good mappings for these, so just fallthrough to generic fail.
    case cr::File::FILE_ERROR_FAILED:
      return ERR_FAILED;
  }
  CR_NOTREACHED();
  return ERR_FAILED;
}

}  // namespace crnet
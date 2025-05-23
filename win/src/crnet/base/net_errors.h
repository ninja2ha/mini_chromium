// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_NET_ERRORS_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_NET_ERRORS_H_

#include <string>
#include <vector>

#include "crbase/logging.h"
#include "crbase/files/file.h"
#include "crbase/build_platform.h"
#include "crnet/base/net_export.h"

namespace crnet {

// Error values are negative.
enum Error {
  // No error. Change NetError.template after changing value.
  OK = 0,

#define NET_ERROR(label, value) ERR_ ## label = value,
#include "crnet/base/net_error_list.h"
#undef NET_ERROR

  // The value of the first certificate error code.
  ERR_CERT_BEGIN = ERR_CERT_COMMON_NAME_INVALID,
};

// Returns a textual representation of the error code for logging purposes.
CRNET_EXPORT std::string ErrorToString(int error);

// Same as above, but leaves off the leading "net::".
CRNET_EXPORT std::string ErrorToShortString(int error);

// Returns true if |error| is a certificate error code. Note this does not
// include errors for client certificates.
CRNET_EXPORT bool IsCertificateError(int error);

// Returns true if |error| is a client certificate authentication error. This
// does not include ERR_SSL_PROTOCOL_ERROR which may also signal a bad client
// certificate.
CRNET_EXPORT bool IsClientCertificateError(int error);

// Returns true if |error| is an error from hostname resolution.
CRNET_EXPORT bool IsHostnameResolutionError(int error);

// Map system error code to Error.
CRNET_EXPORT Error MapSystemError(cr::logging::SystemErrorCode os_error);

// A convenient function to translate file error to net error code.
CRNET_EXPORT Error FileErrorToNetError(cr::File::Error file_error);

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_NET_ERRORS_H_
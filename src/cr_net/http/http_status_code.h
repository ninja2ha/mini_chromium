// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_HTTP_HTTP_STATUS_CODE_H_
#define MINI_CHROMIUM_SRC_CRNET_HTTP_HTTP_STATUS_CODE_H_

#include "cr_net/net_export.h"

namespace crnet {

// HTTP status codes.
enum HttpStatusCode {

#define HTTP_STATUS(label, code, reason) HTTP_ ## label = code,
#include "cr_net/http/http_status_code_list.h"
#undef HTTP_STATUS

};

// Returns the corresponding HTTP status description to use in the Reason-Phrase
// field in an HTTP response for given |code|. It's based on the IANA HTTP
// Status Code Registry.
// http://www.iana.org/assignments/http-status-codes/http-status-codes.xml
//
// This function may not cover all codes defined in the IANA registry. It
// returns an empty string (or crash in debug build) for status codes which are
// not yet covered or just invalid. Please extend it when needed.
CRNET_EXPORT const char* GetHttpReasonPhrase(HttpStatusCode code);

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_HTTP_HTTP_STATUS_CODE_H_
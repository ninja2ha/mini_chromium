// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/http/http_status_code.h"

#include <ostream>

#include "cr_base/logging/logging.h"

namespace crnet {

const char* GetHttpReasonPhrase(HttpStatusCode code) {
  switch (code) {

#define HTTP_STATUS(label, code, reason) case HTTP_ ## label: return reason;
#include "cr_net/http/http_status_code_list.h"
#undef HTTP_STATUS

    default:
      CR_NOTREACHED() << "unknown HTTP status code " << code;
  }

  return "";
}

}  // namespace crnet
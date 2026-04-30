// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/http/http_raw_request_headers.h"

#include "cr_base/strings/string_piece.h"

namespace crnet {

HttpRawRequestHeaders::HttpRawRequestHeaders() = default;
HttpRawRequestHeaders::HttpRawRequestHeaders(HttpRawRequestHeaders&&) = default;
HttpRawRequestHeaders& HttpRawRequestHeaders::operator=(
    HttpRawRequestHeaders&&) = default;
HttpRawRequestHeaders::~HttpRawRequestHeaders() = default;

void HttpRawRequestHeaders::Add(cr::StringPiece key,
                                cr::StringPiece value) {
  headers_.emplace_back(std::string(key), std::string(value));
}

bool HttpRawRequestHeaders::FindHeaderForTest(cr::StringPiece key,
                                              std::string* value) const {
  for (const auto& entry : headers_) {
    if (entry.first == key) {
      *value = entry.second;
      return true;
    }
  }
  return false;
}

}  // namespace crnet

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/http/http_request_headers.h"

#include <utility>

#include "cr_base/logging/logging.h"
#include "cr_base/strings/strcat.h"
#include "cr_base/strings/string_split.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/strings/stringprintf.h"
///#include "net/base/escape.h"
#include "cr_net/http/http_util.h"

namespace crnet {

const char HttpRequestHeaders::kConnectMethod[] = "CONNECT";
const char HttpRequestHeaders::kDeleteMethod[] = "DELETE";
const char HttpRequestHeaders::kGetMethod[] = "GET";
const char HttpRequestHeaders::kHeadMethod[] = "HEAD";
const char HttpRequestHeaders::kOptionsMethod[] = "OPTIONS";
const char HttpRequestHeaders::kPatchMethod[] = "PATCH";
const char HttpRequestHeaders::kPostMethod[] = "POST";
const char HttpRequestHeaders::kPutMethod[] = "PUT";
const char HttpRequestHeaders::kTraceMethod[] = "TRACE";
const char HttpRequestHeaders::kTrackMethod[] = "TRACK";
const char HttpRequestHeaders::kAccept[] = "Accept";
const char HttpRequestHeaders::kAcceptCharset[] = "Accept-Charset";
const char HttpRequestHeaders::kAcceptEncoding[] = "Accept-Encoding";
const char HttpRequestHeaders::kAcceptLanguage[] = "Accept-Language";
const char HttpRequestHeaders::kAuthorization[] = "Authorization";
const char HttpRequestHeaders::kCacheControl[] = "Cache-Control";
const char HttpRequestHeaders::kConnection[] = "Connection";
const char HttpRequestHeaders::kContentLength[] = "Content-Length";
const char HttpRequestHeaders::kContentType[] = "Content-Type";
const char HttpRequestHeaders::kCookie[] = "Cookie";
const char HttpRequestHeaders::kHost[] = "Host";
const char HttpRequestHeaders::kIfMatch[] = "If-Match";
const char HttpRequestHeaders::kIfModifiedSince[] = "If-Modified-Since";
const char HttpRequestHeaders::kIfNoneMatch[] = "If-None-Match";
const char HttpRequestHeaders::kIfRange[] = "If-Range";
const char HttpRequestHeaders::kIfUnmodifiedSince[] = "If-Unmodified-Since";
const char HttpRequestHeaders::kOrigin[] = "Origin";
const char HttpRequestHeaders::kPragma[] = "Pragma";
const char HttpRequestHeaders::kProxyAuthorization[] = "Proxy-Authorization";
const char HttpRequestHeaders::kProxyConnection[] = "Proxy-Connection";
const char HttpRequestHeaders::kRange[] = "Range";
const char HttpRequestHeaders::kReferer[] = "Referer";
const char HttpRequestHeaders::kTransferEncoding[] = "Transfer-Encoding";
const char HttpRequestHeaders::kUserAgent[] = "User-Agent";

HttpRequestHeaders::HeaderKeyValuePair::HeaderKeyValuePair() = default;

HttpRequestHeaders::HeaderKeyValuePair::HeaderKeyValuePair(
    const cr::StringPiece& key,
    const cr::StringPiece& value)
    : key(key.data(), key.size()), value(value.data(), value.size()) {}

HttpRequestHeaders::Iterator::Iterator(const HttpRequestHeaders& headers)
    : started_(false),
      curr_(headers.headers_.begin()),
      end_(headers.headers_.end()) {}

HttpRequestHeaders::Iterator::~Iterator() = default;

bool HttpRequestHeaders::Iterator::GetNext() {
  if (!started_) {
    started_ = true;
    return curr_ != end_;
  }

  if (curr_ == end_)
    return false;

  ++curr_;
  return curr_ != end_;
}

HttpRequestHeaders::HttpRequestHeaders() = default;
HttpRequestHeaders::HttpRequestHeaders(const HttpRequestHeaders& other) =
    default;
HttpRequestHeaders::HttpRequestHeaders(HttpRequestHeaders&& other) = default;
HttpRequestHeaders::~HttpRequestHeaders() = default;

HttpRequestHeaders& HttpRequestHeaders::operator=(
    const HttpRequestHeaders& other) = default;
HttpRequestHeaders& HttpRequestHeaders::operator=(HttpRequestHeaders&& other) =
    default;

bool HttpRequestHeaders::GetHeader(const cr::StringPiece& key,
                                   std::string* out) const {
  auto it = FindHeader(key);
  if (it == headers_.end())
    return false;
  out->assign(it->value);
  return true;
}

void HttpRequestHeaders::Clear() {
  headers_.clear();
}

void HttpRequestHeaders::SetHeader(const cr::StringPiece& key,
                                   const cr::StringPiece& value) {
  // Invalid header names or values could mean clients can attach
  // browser-internal headers.
  CR_CHECK(HttpUtil::IsValidHeaderName(key)) << key;
  CR_CHECK(HttpUtil::IsValidHeaderValue(value)) << key << ":" << value;
  SetHeaderInternal(key, value);
}

void HttpRequestHeaders::SetHeaderIfMissing(const cr::StringPiece& key,
                                            const cr::StringPiece& value) {
  // Invalid header names or values could mean clients can attach
  // browser-internal headers.
  CR_CHECK(HttpUtil::IsValidHeaderName(key));
  CR_CHECK(HttpUtil::IsValidHeaderValue(value));
  auto it = FindHeader(key);
  if (it == headers_.end())
    headers_.push_back(HeaderKeyValuePair(key, value));
}

void HttpRequestHeaders::RemoveHeader(const cr::StringPiece& key) {
  auto it = FindHeader(key);
  if (it != headers_.end())
    headers_.erase(it);
}

void HttpRequestHeaders::AddHeaderFromString(
    const cr::StringPiece& header_line) {
  CR_DCHECK(std::string::npos == header_line.find("\r\n"))
      << "\"" << header_line << "\" contains CRLF.";

  const std::string::size_type key_end_index = header_line.find(":");
  if (key_end_index == std::string::npos) {
    CR_DLOG(Fatal) << "\"" << header_line << "\" is missing colon delimiter.";
    return;
  }

  if (key_end_index == 0) {
    CR_DLOG(Fatal) << "\"" << header_line << "\" is missing header key.";
    return;
  }

  const cr::StringPiece header_key(header_line.data(), key_end_index);
  if (!HttpUtil::IsValidHeaderName(header_key)) {
    CR_DLOG(Fatal) << "\"" << header_line << "\" has invalid header key.";
    return;
  }

  const std::string::size_type value_index = key_end_index + 1;

  if (value_index < header_line.size()) {
    cr::StringPiece header_value(header_line.data() + value_index,
                                 header_line.size() - value_index);
    header_value = HttpUtil::TrimLWS(header_value);
    if (!HttpUtil::IsValidHeaderValue(header_value)) {
      CR_DLOG(Fatal) << "\"" << header_line << "\" has invalid header value.";
      return;
    }
    SetHeader(header_key, header_value);
  } else if (value_index == header_line.size()) {
    SetHeader(header_key, "");
  } else {
    CR_NOTREACHED();
  }
}

void HttpRequestHeaders::AddHeadersFromString(
    const cr::StringPiece& headers) {
  for (const cr::StringPiece& header : cr::SplitStringPieceUsingSubstr(
           headers, "\r\n", cr::TRIM_WHITESPACE, cr::SPLIT_WANT_NONEMPTY)) {
    AddHeaderFromString(header);
  }
}

void HttpRequestHeaders::MergeFrom(const HttpRequestHeaders& other) {
  for (auto it = other.headers_.begin(); it != other.headers_.end(); ++it) {
    SetHeader(it->key, it->value);
  }
}

std::string HttpRequestHeaders::ToString() const {
  std::string output;
  for (auto it = headers_.begin(); it != headers_.end(); ++it) {
    cr::StringAppendF(&output, "%s: %s\r\n", it->key.c_str(),
                      it->value.c_str());
  }
  output.append("\r\n");
  return output;
}

HttpRequestHeaders::HeaderVector::iterator HttpRequestHeaders::FindHeader(
    const cr::StringPiece& key) {
  for (auto it = headers_.begin(); it != headers_.end(); ++it) {
    if (cr::EqualsCaseInsensitiveASCII(key, it->key))
      return it;
  }

  return headers_.end();
}

HttpRequestHeaders::HeaderVector::const_iterator HttpRequestHeaders::FindHeader(
    const cr::StringPiece& key) const {
  for (auto it = headers_.begin(); it != headers_.end(); ++it) {
    if (cr::EqualsCaseInsensitiveASCII(key, it->key))
      return it;
  }

  return headers_.end();
}

void HttpRequestHeaders::SetHeaderInternal(const cr::StringPiece& key,
                                           const cr::StringPiece& value) {
  auto it = FindHeader(key);
  if (it != headers_.end())
    it->value.assign(value.data(), value.size());
  else
    headers_.push_back(HeaderKeyValuePair(key, value));
}

}  // namespace crnet
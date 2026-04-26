// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_ipc/named_channel.h"

#include "cr_base/logging/logging.h"
#include "cr_base/strings/utf_string_conversions.h"

namespace cripc {

NamedChannel::NamedChannel(const Options& options) {
  server_endpoint_ = ChannelServerEndpoint(
      CreateServerEndpoint(options, &server_name_));
}

NamedChannel::NamedChannel(NamedChannel&& other) =
    default;

NamedChannel::~NamedChannel() = default;

NamedChannel& NamedChannel::operator=(
    NamedChannel&& other) = default;

// static
NamedChannel::ServerName NamedChannel::ServerNameFromUTF8(
    cr::StringPiece name) {
#if defined(MINI_CHROMIUM_OS_WIN)
  return cr::UTF8ToWide(name);
#else
  return name.as_string();
#endif
}

// static
ChannelEndpoint NamedChannel::ConnectToServer(const ServerName& server_name) {
  CR_DCHECK(!server_name.empty());
  return CreateClientEndpoint(server_name);
}

}  // namespace cripc
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/named_platform_channel.h"

#include "crbase/logging/logging.h"
#include "crbase/strings/utf_string_conversions.h"

namespace mojo {

NamedPlatformChannel::NamedPlatformChannel(const Options& options) {
  server_endpoint_ = PlatformChannelServerEndpoint(
      CreateServerEndpoint(options, &server_name_));
}

NamedPlatformChannel::NamedPlatformChannel(NamedPlatformChannel&& other) =
    default;

NamedPlatformChannel::~NamedPlatformChannel() = default;

NamedPlatformChannel& NamedPlatformChannel::operator=(
    NamedPlatformChannel&& other) = default;

// static
NamedPlatformChannel::ServerName NamedPlatformChannel::ServerNameFromUTF8(
    cr::StringPiece name) {
#if defined(MINI_CHROMIUM_OS_WIN)
  return cr::UTF8ToWide(name);
#else
  return name.as_string();
#endif
}

// static
PlatformChannelEndpoint NamedPlatformChannel::ConnectToServer(
    const ServerName& server_name) {
  CR_DCHECK(!server_name.empty());
  return CreateClientEndpoint(server_name);
}

}  // namespace mojo
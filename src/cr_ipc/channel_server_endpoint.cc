// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_ipc/channel_server_endpoint.h"

namespace cripc {

ChannelServerEndpoint::ChannelServerEndpoint() = default;

ChannelServerEndpoint::ChannelServerEndpoint(
    ChannelServerEndpoint&& other) = default;

ChannelServerEndpoint::ChannelServerEndpoint(
    PlatformHandle handle)
    : handle_(std::move(handle)) {}

ChannelServerEndpoint::~ChannelServerEndpoint() = default;

ChannelServerEndpoint& ChannelServerEndpoint::operator=(
    ChannelServerEndpoint&& other) = default;

void ChannelServerEndpoint::reset() {
  handle_.reset();
}

ChannelServerEndpoint ChannelServerEndpoint::Clone() const {
  return ChannelServerEndpoint(handle_.Clone());
}

}  // namespace mojo
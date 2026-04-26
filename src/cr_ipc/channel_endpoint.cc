// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_ipc/channel_endpoint.h"

namespace cripc {

ChannelEndpoint::ChannelEndpoint() = default;

ChannelEndpoint::ChannelEndpoint(
    ChannelEndpoint&& other) = default;

ChannelEndpoint::ChannelEndpoint(PlatformHandle handle)
    : handle_(std::move(handle)) {}

ChannelEndpoint::~ChannelEndpoint() = default;

ChannelEndpoint& ChannelEndpoint::operator=(
    ChannelEndpoint&& other) = default;

void ChannelEndpoint::reset() {
  handle_.reset();
}

ChannelEndpoint ChannelEndpoint::Clone() const {
  return ChannelEndpoint(handle_.Clone());
}

}  // namespace mojo
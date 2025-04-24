// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/ipc_channel.h"

namespace cripc {

// static
std::unique_ptr<Channel> Channel::CreateClient(
    const ChannelHandle& channel_handle,
    Channel::Delegate* delegate) {
  return Channel::Create(channel_handle, Channel::MODE_CLIENT, delegate);
}

// static
std::unique_ptr<Channel> Channel::CreateNamedServer(
    const ChannelHandle& channel_handle,
    Channel::Delegate* delegate) {
  return Channel::Create(channel_handle, Channel::MODE_NAMED_SERVER, delegate);
}

// static
std::unique_ptr<Channel> Channel::CreateNamedClient(
    const ChannelHandle& channel_handle,
    Channel::Delegate* delegate) {
  return Channel::Create(channel_handle, Channel::MODE_NAMED_CLIENT, delegate);
}

// static
std::unique_ptr<Channel> Channel::CreateServer(
    const ChannelHandle& channel_handle,
    Channel::Delegate* delegate) {
  return Channel::Create(channel_handle, Channel::MODE_SERVER, delegate);
}

Channel::~Channel() {
}

bool Channel::IsSendThreadSafe() const {
  return false;
}

}  // namespace cripc

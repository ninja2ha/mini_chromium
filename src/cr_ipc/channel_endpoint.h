// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRIPC_CHANNEL_END_POINT_H_
#define MINI_CHROMIUM_SRC_CRIPC_CHANNEL_END_POINT_H_

#include "cr_ipc/ipc_export.h"
#include "cr_ipc/platform_handle.h"

#include "cr_build/compiler_specific.h"

namespace cripc {

// A PlatformHandle with a little extra type information to convey that it's
// a channel endpoint, i.e. a handle that can be used to send or receive
// invitations as |MOJO_INVITATION_TRANSPORT_TYPE_CHANNEL| to a remote
// PlatformChannelEndpoint.
class CRIPC_EXPORT ChannelEndpoint {
 public:
  ChannelEndpoint(const ChannelEndpoint&) = delete;
  ChannelEndpoint& operator=(const ChannelEndpoint&) = delete;

  ChannelEndpoint();
  ChannelEndpoint(ChannelEndpoint&& other);
  explicit ChannelEndpoint(PlatformHandle handle);
  ~ChannelEndpoint();

  ChannelEndpoint& operator=(ChannelEndpoint&& other);

  bool is_valid() const { return handle_.is_valid(); }
  void reset();
  ChannelEndpoint Clone() const;

  const PlatformHandle& platform_handle() const { return handle_; }

  PlatformHandle TakePlatformHandle() CR_WARN_UNUSED_RESULT {
    return std::move(handle_);
  }

 private:
  PlatformHandle handle_;
};

}  // namespace cripc

#endif  // MINI_CHROMIUM_SRC_CRIPC_CHANNEL_END_POINT_H_
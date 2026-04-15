// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_PLATFORM_PLATFORM_CHANNEL_SERVER_ENDPOINT_H_
#define MOJO_PUBLIC_CPP_PLATFORM_PLATFORM_CHANNEL_SERVER_ENDPOINT_H_

#include "cripc/ipc_export.h"
#include "cripc/platform_handle.h"
#include "crbuild/compiler_specific.h"

namespace mojo {

// A PlatformHandle with a little extra type information to convey that it's
// a channel server endpoint, i.e. a handle that can be used to send invitations
// as |MOJO_INVITATION_TRANSPORT_TYPE_CHANNEL_SERVER| to a remote
// PlatformChannelEndpoint.
class CRIPC_EXPORT PlatformChannelServerEndpoint {
 public:
  PlatformChannelServerEndpoint();
  PlatformChannelServerEndpoint(PlatformChannelServerEndpoint&& other);
  explicit PlatformChannelServerEndpoint(PlatformHandle handle);
  ~PlatformChannelServerEndpoint();

  PlatformChannelServerEndpoint& operator=(
      PlatformChannelServerEndpoint&& other);

  bool is_valid() const { return handle_.is_valid(); }
  void reset();
  PlatformChannelServerEndpoint Clone() const;

  const PlatformHandle& platform_handle() const { return handle_; }

  PlatformHandle TakePlatformHandle() CR_WARN_UNUSED_RESULT {
    return std::move(handle_);
  }

 private:
  PlatformHandle handle_;
};

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_PLATFORM_PLATFORM_CHANNEL_SERVER_ENDPOINT_H_
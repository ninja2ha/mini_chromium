// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_TAG_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_TAG_H_

#include "cr_net/net_export.h"
#include "cr_build/build_config.h"
#include "cr_net/socket/socket_descriptor.h"

namespace crnet {

// SocketTag represents a tag that can be applied to a socket. Currently only
// implemented for Android, it facilitates assigning a Android TrafficStats tag
// and UID to a socket so that future network data usage by the socket is
// attributed to the tag and UID that the socket is tagged with.
//
// This class is small (<=64-bits) and contains only POD to facilitate default
// copy and assignment operators so that it can easily be passed by value.
class CRNET_EXPORT SocketTag {
 public:
  SocketTag() {}
  ~SocketTag() {}

  bool operator<(const SocketTag& other) const;
  bool operator==(const SocketTag& other) const;
  bool operator!=(const SocketTag& other) const { return !(*this == other); }

  // Apply this tag to |socket|.
  void Apply(SocketDescriptor socket) const;
  // Copying and assignment are allowed.
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_SOCKET_TAG_H_
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRIPC_CONNECTION_PARAMS_H_
#define MINI_CHROMIUM_SRC_CRIPC_CONNECTION_PARAMS_H_

#include "cr_ipc/channel_endpoint.h"
#include "cr_ipc/channel_server_endpoint.h"
#include "cr_ipc/ipc_export.h"
#include "cr_build/build_config.h"

namespace cripc {

// A set of parameters used when establishing a connection to another process.
class CRIPC_EXPORT ConnectionParams {
 public:
  ConnectionParams(const ConnectionParams&) = delete;
  ConnectionParams& operator=(const ConnectionParams&) = delete;

  ConnectionParams();
  explicit ConnectionParams(ChannelEndpoint endpoint);
  explicit ConnectionParams(ChannelServerEndpoint server_endpoint);
  ConnectionParams(ConnectionParams&&);
  ~ConnectionParams();

  ConnectionParams& operator=(ConnectionParams&&);

  const ChannelEndpoint& endpoint() const { return endpoint_; }
  const ChannelServerEndpoint& server_endpoint() const {
    return server_endpoint_;
  }

  ChannelEndpoint TakeEndpoint() { return std::move(endpoint_); }

  ChannelServerEndpoint TakeServerEndpoint() {
    return std::move(server_endpoint_);
  }

  void set_is_async(bool is_async) { is_async_ = is_async; }
  bool is_async() const { return is_async_; }

  void set_leak_endpoint(bool leak_endpoint) { leak_endpoint_ = leak_endpoint; }
  bool leak_endpoint() const { return leak_endpoint_; }

 private:
  bool is_async_ = false;
  bool leak_endpoint_ = false;
  ChannelEndpoint endpoint_;
  ChannelServerEndpoint server_endpoint_;
};

}  // namespace cripc

#endif  // MINI_CHROMIUM_SRC_CRIPC_CONNECTION_PARAMS_H_
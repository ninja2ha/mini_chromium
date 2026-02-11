// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_CORE_CONNECTION_PARAMS_H_
#define MOJO_CORE_CONNECTION_PARAMS_H_

///#include "mojo/core/system_impl_export.h"
#include "cripc/platform_channel_endpoint.h"
#include "cripc/platform_channel_server_endpoint.h"
#include "cripc/ipc_export.h"
#include "crbuild/build_config.h"

namespace mojo {
namespace core {

// A set of parameters used when establishing a connection to another process.
class CRIPC_EXPORT ConnectionParams {
 public:
  ConnectionParams(const ConnectionParams&) = delete;
  ConnectionParams& operator=(const ConnectionParams&) = delete;

  ConnectionParams();
  explicit ConnectionParams(PlatformChannelEndpoint endpoint);
  explicit ConnectionParams(PlatformChannelServerEndpoint server_endpoint);
  ConnectionParams(ConnectionParams&&);
  ~ConnectionParams();

  ConnectionParams& operator=(ConnectionParams&&);

  const PlatformChannelEndpoint& endpoint() const { return endpoint_; }
  const PlatformChannelServerEndpoint& server_endpoint() const {
    return server_endpoint_;
  }

  PlatformChannelEndpoint TakeEndpoint() { return std::move(endpoint_); }

  PlatformChannelServerEndpoint TakeServerEndpoint() {
    return std::move(server_endpoint_);
  }

  void set_is_async(bool is_async) { is_async_ = is_async; }
  bool is_async() const { return is_async_; }

  void set_leak_endpoint(bool leak_endpoint) { leak_endpoint_ = leak_endpoint; }
  bool leak_endpoint() const { return leak_endpoint_; }

 private:
  bool is_async_ = false;
  bool leak_endpoint_ = false;
  PlatformChannelEndpoint endpoint_;
  PlatformChannelServerEndpoint server_endpoint_;
};

}  // namespace core
}  // namespace mojo

#endif  // MOJO_CORE_CONNECTION_PARAMS_H_
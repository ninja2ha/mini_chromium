// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/socket/client_socket_factory.h"

#include <utility>

#include "crbase/memory/lazy_instance.h"
#include "crnet/socket/tcp/tcp_client_socket.h"
#include "crnet/socket/udp/udp_client_socket.h"

namespace crnet {
namespace {

class DefaultClientSocketFactory : public ClientSocketFactory {
 public:
  DefaultClientSocketFactory() = default;

  // Note: This code never runs, as the factory is defined as a Leaky singleton.
  ~DefaultClientSocketFactory() override = default;

  std::unique_ptr<DatagramClientSocket> CreateDatagramClientSocket(
      DatagramSocket::BindType bind_type) override {
    return std::make_unique<UDPClientSocket>(bind_type);
  }

  std::unique_ptr<TransportClientSocket> CreateTransportClientSocket(
      const AddressList& addresses,
      std::unique_ptr<SocketPerformanceWatcher> socket_performance_watcher
      ) override {
    return std::make_unique<TCPClientSocket>(
        addresses, std::move(socket_performance_watcher));
  }
};

static cr::LazyInstance<DefaultClientSocketFactory>::Leaky
    g_default_client_socket_factory = LAZY_INSTANCE_INITIALIZER;

}  // namespace

// static
ClientSocketFactory* ClientSocketFactory::GetDefaultFactory() {
  return g_default_client_socket_factory.Pointer();
}

}  // namespace crnet
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_CLIENT_SOCKET_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_CLIENT_SOCKET_H_

#include <stdint.h>

#include "cr_net/net_export.h"
#include "cr_net/socket/udp/datagram_client_socket.h"
#include "cr_net/socket/udp/udp_socket.h"
///#include "cr_net/traffic_annotation/network_traffic_annotation.h"

namespace crnet {

///class NetLog;
///struct NetLogSource;

// A client socket that uses UDP as the transport layer.
class CRNET_EXPORT UDPClientSocket : public DatagramClientSocket {
 public:
  UDPClientSocket(const UDPClientSocket&) = delete;
  UDPClientSocket& operator=(const UDPClientSocket&) = delete;

  UDPClientSocket(DatagramSocket::BindType bind_type/*,
                  net::NetLog* net_log,
                  const net::NetLogSource& source*/);
  ~UDPClientSocket() override;

  // DatagramClientSocket implementation.
  int Connect(const IPEndPoint& address) override;
  int ConnectUsingNetwork(NetworkChangeNotifier::NetworkHandle network,
                          const IPEndPoint& address) override;
  int ConnectUsingDefaultNetwork(const IPEndPoint& address) override;
  NetworkChangeNotifier::NetworkHandle GetBoundNetwork() const override;
  void ApplySocketTag(const SocketTag& tag) override;
  int Read(cr::IOBuffer* buf,
           int buf_len,
           CompletionOnceCallback callback) override;
  int Write(cr::IOBuffer* buf,
            int buf_len,
            CompletionOnceCallback callback/*,
            const NetworkTrafficAnnotationTag& traffic_annotation*/) override;

  int WriteAsync(
      const char* buffer,
      size_t buf_len,
      CompletionOnceCallback callback/*,
      const NetworkTrafficAnnotationTag& traffic_annotation*/) override;
  int WriteAsync(
      DatagramBuffers buffers,
      CompletionOnceCallback callback/*,
      const NetworkTrafficAnnotationTag& traffic_annotation*/) override;

  DatagramBuffers GetUnwrittenBuffers() override;

  void Close() override;
  int GetPeerAddress(IPEndPoint* address) const override;
  int GetLocalAddress(IPEndPoint* address) const override;
  // Switch to use non-blocking IO. Must be called right after construction and
  // before other calls.
  void UseNonBlockingIO() override;
  int SetReceiveBufferSize(int32_t size) override;
  int SetSendBufferSize(int32_t size) override;
  int SetDoNotFragment() override;
  void SetMsgConfirm(bool confirm) override;
  ///const NetLogWithSource& NetLog() const override;
  void EnableRecvOptimization() override;

  void SetWriteAsyncEnabled(bool enabled) override;
  bool WriteAsyncEnabled() override;
  void SetMaxPacketSize(size_t max_packet_size) override;
  void SetWriteMultiCoreEnabled(bool enabled) override;
  void SetSendmmsgEnabled(bool enabled) override;
  void SetWriteBatchingActive(bool active) override;
  int SetMulticastInterface(uint32_t interface_index) override;
  void SetIOSNetworkServiceType(int ios_network_service_type) override;

 private:
  UDPSocket socket_;
  NetworkChangeNotifier::NetworkHandle network_;
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_CLIENT_SOCKET_H_
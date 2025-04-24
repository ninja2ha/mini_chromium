// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_CLIENT_SOCKET_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_CLIENT_SOCKET_H_

#include <stdint.h>

#include "crbase/build_platform.h"
#include "crnet/base/net_export.h"
#include "crnet/socket/udp/datagram_client_socket.h"
#include "crnet/socket/udp/udp_socket.h"

namespace crnet {

// A client socket that uses UDP as the transport layer.
class CRNET_EXPORT_PRIVATE UDPClientSocket : public DatagramClientSocket {
 public:
  UDPClientSocket(const UDPClientSocket&) = delete;
  UDPClientSocket& operator=(const UDPClientSocket&) = delete;

  UDPClientSocket(DatagramSocket::BindType bind_type);
  ~UDPClientSocket() override;

  // DatagramClientSocket implementation.
  int Connect(const IPEndPoint& address) override;
#if defined(MINI_CHROMIUM_OS_POSIX)
  int ConnectUsingNetwork(NetworkChangeNotifier::NetworkHandle network,
                          const IPEndPoint& address) override;
  int ConnectUsingDefaultNetwork(const IPEndPoint& address) override;
  NetworkChangeNotifier::NetworkHandle GetBoundNetwork() const override;
  void ApplySocketTag(const SocketTag& tag) override;
#endif
  int Read(cr::IOBuffer* buf,
           int buf_len,
           CompletionOnceCallback callback) override;
  int Write(cr::IOBuffer* buf,
            int buf_len,
            CompletionOnceCallback callback) override;

#if defined(MINI_CHROMIUM_OS_POSIX)
  int WriteAsync(
      const char* buffer,
      size_t buf_len,
      CompletionOnceCallback callback,
      const NetworkTrafficAnnotationTag& traffic_annotation) override;
  int WriteAsync(
      DatagramBuffers buffers,
      CompletionOnceCallback callback,
      const NetworkTrafficAnnotationTag& traffic_annotation) override;

  DatagramBuffers GetUnwrittenBuffers() override;

  void SetWriteAsyncEnabled(bool enabled) override;
  bool WriteAsyncEnabled() override;
  void SetMaxPacketSize(size_t max_packet_size) override;
  void SetWriteMultiCoreEnabled(bool enabled) override;
  void SetSendmmsgEnabled(bool enabled) override;
  void SetWriteBatchingActive(bool active) override;
#endif

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
  void EnableRecvOptimization() override;
  int SetMulticastInterface(uint32_t interface_index) override;

 private:
  UDPSocket socket_;
#if defined(MINI_CHROMIUM_OS_POSIX)
  NetworkChangeNotifier::NetworkHandle network_;
#endif
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_UDP_UDP_CLIENT_SOCKET_H_
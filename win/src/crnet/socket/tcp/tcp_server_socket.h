// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SERVER_SOCKET_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SERVER_SOCKET_H_

#include <memory>

#include "crnet/base/completion_once_callback.h"
#include "crnet/base/ip_endpoint.h"
#include "crnet/base/net_export.h"
#include "crnet/socket/socket_descriptor.h"
#include "crnet/socket/tcp/transport_server_socket.h"
#include "crnet/socket/tcp/tcp_socket.h"

namespace crnet {

// A server socket that uses TCP as the transport layer.
class CRNET_EXPORT TCPServerSocket : public TransportServerSocket {
 public:
  TCPServerSocket(const TCPServerSocket&) = delete;
  TCPServerSocket& operator=(const TCPServerSocket&) = delete;

  TCPServerSocket();
  // Adopts the provided socket, which must not be a connected socket.
  explicit TCPServerSocket(std::unique_ptr<TCPSocket> socket);

  ~TCPServerSocket() override;

  // Takes ownership of |socket|, which has been opened, but may or may not be
  // bound or listening. The caller must determine this based on the provenance
  // of the socket and act accordingly. The socket may have connections waiting
  // to be accepted, but must not be actually connected.
  int AdoptSocket(SocketDescriptor socket);

  // crnet::ServerSocket implementation.
  int Listen(const IPEndPoint& address, int backlog) override;
  int GetLocalAddress(IPEndPoint* address) const override;
  int Accept(std::unique_ptr<StreamSocket>* socket,
             CompletionOnceCallback callback) override;

  // Detachs from the current thread, to allow the socket to be transferred to
  // a new thread. Should only be called when the object is no longer used by
  // the old thread.
  void DetachFromThread();

 private:
  // Converts |accepted_socket_| and stores the result in
  // |output_accepted_socket|.
  // |output_accepted_socket| is untouched on failure. But |accepted_socket_| is
  // set to NULL in any case.
  int ConvertAcceptedSocket(
      int result,
      std::unique_ptr<StreamSocket>* output_accepted_socket);
  // Completion callback for calling TCPSocket::Accept().
  void OnAcceptCompleted(std::unique_ptr<StreamSocket>* output_accepted_socket,
                         CompletionOnceCallback forward_callback,
                         int result);

  std::unique_ptr<TCPSocket> socket_;

  std::unique_ptr<TCPSocket> accepted_socket_;
  IPEndPoint accepted_address_;
  bool pending_accept_;
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SERVER_SOCKET_H_
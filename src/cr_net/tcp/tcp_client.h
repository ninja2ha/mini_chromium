// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CLIENT_H_
#define MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CLIENT_H_

#include <memory>

#include "cr_base/memory/weak_ptr.h"

#include "cr_net/net_export.h"
#include "cr_net/socket/tcp/transport_client_socket.h"
#include "cr_net/tcp/tcp_connection.h"

namespace crnet {

class CRNET_EXPORT TCPClient {
 public:
  // Delegate to handle tcp events. Beware that it is not safe to destroy the 
  // TCPClient in any of these callbacks.
  class Delegate {
   public:
    virtual ~Delegate() {}

    // |rv|: a net error code.
    virtual void OnConnect(int rv) = 0;

    // returns length of hints. or a net error code. which value is less than 0.
    virtual int OnReceiveData(const char* data, int data_len) = 0;

    virtual void OnClose() = 0;
  };

  TCPClient(const TCPClient&) = delete;
  TCPClient& operator=(const TCPClient&) = delete;

  TCPClient(std::unique_ptr<TransportClientSocket> client_socket,
            TCPClient::Delegate* delegate);
  ~TCPClient();

  // Sends the provided data directly
  void SendData(const std::string& data);
  void Close();

  void SetReceiveBufferSize(int32_t size);
  void SetSendBufferSize(int32_t size); 

  TCPConnection* connection() const {
    return connection_.get();
  };

 private:
  void DoConnect();
  void HandleConnectResult(int rv);

  void DoReadLoop();
  void OnReadCompleted(int rv);
  int HandleReadResult(int rv);

  void DoWriteLoop();
  void OnWriteCompleted(int rv);
  int HandleWriteResult(int rv);

  bool HasClosedConnection();

  std::unique_ptr<TransportClientSocket> socket_;
  std::unique_ptr<TCPConnection> connection_;
  TCPClient::Delegate * const delegate_ = nullptr;

  cr::WeakPtrFactory<TCPClient> weak_ptr_factory_{ this };
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CLIENT_H_
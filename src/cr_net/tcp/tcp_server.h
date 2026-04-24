// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_TCP_TCP_SERVER_H_
#define MINI_CHROMIUM_SRC_CRNET_TCP_TCP_SERVER_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cr_base/memory/weak_ptr.h"
#include "cr_base/strings/string_piece.h"

#include "cr_net/net_export.h"
#include "cr_net/base/ip_endpoint.h"
#include "cr_net/socket/tcp/stream_socket.h"
#include "cr_net/socket/tcp/transport_server_socket.h"
#include "cr_net/tcp/tcp_connection.h"

namespace crnet {

class CRNET_EXPORT TCPServer {
 public:
  // Delegate to handle tcp events. Beware that it is not safe to destroy the 
  // TCPServer in any of these callbacks.
  class Delegate {
   public:
    virtual ~Delegate() {}

    virtual void OnAccept(TCPConnection* connection) = 0;
    virtual int OnReceiveData(
        TCPConnection* connection, const char* data, int data_len) = 0;
    virtual void OnClose(TCPConnection* connection) = 0;
  };

  TCPServer(const TCPServer&) = delete;
  TCPServer& operator=(const TCPServer&) = delete;

  // Instantiates a tcp server with |server_socket| which already started
  // listening, but not accepting.  This constructor schedules accepting
  // connections asynchronously in case when |delegate| is not ready to get
  // callbacks yet.
  TCPServer(std::unique_ptr<TransportServerSocket> server_socket,
            TCPServer::Delegate* delegate);
  ~TCPServer();

  // Sends the provided data directly to the given connection.
  void SendData(TCPConnection::Id connection_id, const std::string& data);
  // Sends the provided data directly to all connections.
  void SendDataToAll(const std::string& data);

  void Close(TCPConnection::Id connection_id);

  void SetReceiveBufferSize(TCPConnection::Id connection_id, int32_t size);
  void SetSendBufferSize(TCPConnection::Id connection_id, int32_t size);

  // Copies the local address to |address|. Returns a network error code.
  int GetLocalAddress(IPEndPoint* address);

 private:
  void DoAcceptLoop();
  void OnAcceptCompleted(int rv);
  int HandleAcceptResult(int rv);

  void DoReadLoop(TCPConnection* connection);
  void OnReadCompleted(TCPConnection::Id connection_id, int rv);
  int HandleReadResult(TCPConnection* connection, int rv);

  void DoWriteLoop(TCPConnection* connection);
  void OnWriteCompleted(TCPConnection::Id connection_id,
                        int rv);
  int HandleWriteResult(TCPConnection* connection, int rv);

  TCPConnection* FindConnection(TCPConnection::Id connection_id);

  // Whether or not Close() has been called during delegate callback processing.
  bool HasClosedConnection(TCPConnection* connection);

  void DestroyClosedConnections();

  const std::unique_ptr<TransportServerSocket> server_socket_;
  std::unique_ptr<StreamSocket> accepted_socket_;
  TCPServer::Delegate* const delegate_ = nullptr;

  TCPConnection::Id last_id_ = 0;
  std::map<TCPConnection::Id, std::unique_ptr<TCPConnection>> id_to_connection_;

  // Vector of connections whose destruction is pending. Connections may have
  // WebSockets with raw pointers to `this`, so should not out live this, but
  // also cannot safely be destroyed synchronously, so on connection close, add
  // a Connection here, and post a task to destroy them.
  std::vector<std::unique_ptr<TCPConnection>> closed_connections_;

  cr::WeakPtrFactory<TCPServer> weak_ptr_factory_{this};
};

}  // namespace crnet

#endif // MINI_CHROMIUM_SRC_CRNET_TCP_TCP_SERVER_H_
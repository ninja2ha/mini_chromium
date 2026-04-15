// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40284755): Remove this and spanify to fix the errors.
#pragma allow_unsafe_buffers
#endif

#include "cr_net/tcp/tcp_server.h"

#include <utility>

#include "cr_base/logging/logging.h"
#include "cr_base/location.h"
#include "cr_base/functional/bind.h"
#include "cr_event/single_thread_task_runner.h"
#include "cr_event/threading/thread_task_runner_handle.h"
#include "cr_net/base/net_errors.h"
#include "cr_net/socket/tcp/transport_server_socket.h"
#include "cr_net/socket/tcp/stream_socket.h"
#include "cr_net/socket/tcp/tcp_server_socket.h"
#include "cr_net/tcp/tcp_connection.h"
#include "cr_build/compiler_specific.h"
#include "cr_build/build_config.h"

namespace crnet {

TCPServer::TCPServer(std::unique_ptr<TransportServerSocket> server_socket,
                     TCPServer::Delegate* delegate)
    : server_socket_(std::move(server_socket)), delegate_(delegate) {
  CR_DCHECK(server_socket_);
  // Start accepting connections in next run loop in case when delegate is not
  // ready to get callbacks.
  cr::ThreadTaskRunnerHandle::Get()->PostTask(
      CR_FROM_HERE, cr::BindOnce(&TCPServer::DoAcceptLoop,
                                 weak_ptr_factory_.GetWeakPtr()));
}

TCPServer::~TCPServer() = default;

void TCPServer::SendData(TCPConnection::ID connection_id,
                         const std::string& data) {
  TCPConnection* connection = FindConnection(connection_id);
  if (connection == nullptr)
    return;

  bool writing_in_progress = !connection->write_buf()->IsEmpty();
  if (connection->write_buf()->Append(data) && !writing_in_progress)
    DoWriteLoop(connection);
}

void TCPServer::SendDataToAll(const std::string& data) {
  auto it = id_to_connection_.begin();
  auto it_end = id_to_connection_.end();
  for (; it != it_end; it++) {
    TCPConnection* connection = it->second.get();
    CR_DCHECK(connection);
    if (connection == nullptr)
      continue;

    bool writing_in_progress = !connection->write_buf()->IsEmpty();
    if (connection->write_buf()->Append(data) && !writing_in_progress)
      DoWriteLoop(connection);
  }
}

void TCPServer::Close(TCPConnection::ID connection_id) {
  auto it = id_to_connection_.find(connection_id);
  if (it == id_to_connection_.end())
    return;

  TCPConnection* connection = it->second.get();

  closed_connections_.emplace_back(std::move(it->second));
  id_to_connection_.erase(it);
  delegate_->OnClose(connection);

  // The call stack might have callbacks which still have the pointer of
  // connection. Instead of referencing connection with ID all the time,
  // destroys the connection in next run loop to make sure any pending
  // callbacks in the call stack return. List of closed Connections is owned
  // by `this` in case `this` is destroyed before the task runs. Connections may
  // not outlive `this`.
  cr::ThreadTaskRunnerHandle::Get()->PostTask(
      CR_FROM_HERE, cr::BindOnce(&TCPServer::DestroyClosedConnections,
                                 weak_ptr_factory_.GetWeakPtr()));
}

int TCPServer::GetLocalAddress(IPEndPoint* address) {
  return server_socket_->GetLocalAddress(address);
}

void TCPServer::SetReceiveBufferSize(TCPConnection::ID connection_id, 
                                     int32_t size) {
  TCPConnection* connection = FindConnection(connection_id);
  if (connection)
    connection->read_buf()->set_max_buffer_size(size);
}

void TCPServer::SetSendBufferSize(TCPConnection::ID connection_id, 
                                  int32_t size) {
  TCPConnection* connection = FindConnection(connection_id);
  if (connection)
    connection->write_buf()->set_max_buffer_size(size);
}

void TCPServer::DoAcceptLoop() {
  int rv;
  do {
    rv = server_socket_->Accept(&accepted_socket_,
                                cr::BindOnce(&TCPServer::OnAcceptCompleted,
                                             weak_ptr_factory_.GetWeakPtr()));
    if (rv == ERR_IO_PENDING)
      return;
    rv = HandleAcceptResult(rv);
  } while (rv == OK);
}

void TCPServer::OnAcceptCompleted(int rv) {
  if (HandleAcceptResult(rv) == OK)
    DoAcceptLoop();
}

int TCPServer::HandleAcceptResult(int rv) {
  if (rv < 0) {
    CR_LOG(Error) << "Accept error: rv=" << rv;
    return rv;
  }

  std::unique_ptr<TCPConnection> connection_ptr =
      std::make_unique<TCPConnection>(++last_id_, std::move(accepted_socket_));
  TCPConnection* connection = connection_ptr.get();
  id_to_connection_[connection->id()] = std::move(connection_ptr);
  delegate_->OnAccept(connection);
  if (!HasClosedConnection(connection))
    DoReadLoop(connection);
  return OK;
}

void TCPServer::DoReadLoop(TCPConnection* connection) {
  int rv;
  do {
    TCPConnection::ReadIOBuffer* read_buf = connection->read_buf();
    // Increases read buffer size if necessary.
    if (read_buf->RemainingCapacity() == 0 && !read_buf->IncreaseCapacity()) {
      Close(connection->id());
      return;
    }

    rv = connection->socket()->Read(
        read_buf, read_buf->RemainingCapacity(),
        cr::BindOnce(&TCPServer::OnReadCompleted,
                     weak_ptr_factory_.GetWeakPtr(), connection->id()));
    if (rv == ERR_IO_PENDING)
      return;
    rv = HandleReadResult(connection, rv);
  } while (rv == OK);
}

void TCPServer::OnReadCompleted(TCPConnection::ID connection_id, int rv) {
  TCPConnection* connection = FindConnection(connection_id);
  if (!connection)  // It might be closed right before by write error.
    return;

  if (HandleReadResult(connection, rv) == OK)
    DoReadLoop(connection);
}

int TCPServer::HandleReadResult(TCPConnection* connection, int rv) {
  if (rv <= 0) {
    Close(connection->id());
    return rv == 0 ? ERR_CONNECTION_CLOSED : rv;
  }

  TCPConnection::ReadIOBuffer* read_buf = connection->read_buf();
  read_buf->DidRead(rv);

  // Handles stream data.
  while (!read_buf->readable_bytes().empty()) {
    int handled = delegate_->OnTranslateData(
      connection,
      reinterpret_cast<const char*>(read_buf->readable_bytes().data()),
      static_cast<int>(read_buf->readable_bytes().size()));
    if (handled == 0) {
      break;
    }
    else if (handled < 0) {
      // An error has been occured. Closing the connection.
      Close(connection->id());
      return ERR_CONNECTION_CLOSED;
    }

    read_buf->DidConsume(handled);
    if (HasClosedConnection(connection))
      return ERR_CONNECTION_CLOSED;
  }

  return OK;
}

void TCPServer::DoWriteLoop(TCPConnection* connection) {
  int rv = OK;
  TCPConnection::QueuedWriteIOBuffer* write_buf = connection->write_buf();
  while (rv == OK && write_buf->GetSizeToWrite() > 0) {
    rv = connection->socket()->Write(
        write_buf, write_buf->GetSizeToWrite(),
        cr::BindOnce(&TCPServer::OnWriteCompleted,
                     weak_ptr_factory_.GetWeakPtr(), connection->id()));
    if (rv == ERR_IO_PENDING || rv == OK)
      return;
    rv = HandleWriteResult(connection, rv);
  }
}

void TCPServer::OnWriteCompleted(
    TCPConnection::ID connection_id,
    int rv) {
  TCPConnection* connection = FindConnection(connection_id);
  if (!connection)  // It might be closed right before by read error.
    return;

  if (HandleWriteResult(connection, rv) == OK)
    DoWriteLoop(connection);
}

int TCPServer::HandleWriteResult(TCPConnection* connection, int rv) {
  if (rv < 0) {
    Close(connection->id());
    return rv;
  }

  connection->write_buf()->DidConsume(rv);
  return OK;
}

TCPConnection* TCPServer::FindConnection(TCPConnection::ID connection_id) {
  auto it = id_to_connection_.find(connection_id);
  if (it == id_to_connection_.end())
    return nullptr;
  return it->second.get();
}

// This is called after any delegate callbacks are called to check if Close()
// has been called during callback processing. Using the pointer of connection,
// |connection| is safe here because Close() deletes the connection in next run
// loop.
bool TCPServer::HasClosedConnection(TCPConnection* connection) {
  return FindConnection(connection->id()) != connection;
}

void TCPServer::DestroyClosedConnections() {
  closed_connections_.clear();
}

}  // namespace crnet
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/tcp/tcp_client.h"

#include "cr_base/logging/logging.h"
#include "cr_base/location.h"
#include "cr_base/functional/bind.h"
#include "cr_event/single_thread_task_runner.h"
#include "cr_event/threading/thread_task_runner_handle.h"

namespace crnet {

TCPClient::TCPClient(std::unique_ptr<TransportClientSocket> client_socket,
                     TCPClient::Delegate* delegate) 
    : socket_(std::move(client_socket)), delegate_(delegate) {
  CR_DCHECK(socket_);
  CR_DCHECK(delegate);

  // Start connect in next run loop in case when delegate is not ready to 
  // get callbacks.
  cr::ThreadTaskRunnerHandle::Get()->PostTask(
      CR_FROM_HERE, 
      cr::BindOnce(&TCPClient::DoConnect, weak_ptr_factory_.GetWeakPtr()));
}

TCPClient::~TCPClient() = default;

void TCPClient::SendData(const std::string& data) {
  if (connection_ == nullptr)
    return;

  bool writing_in_progress = !connection_->write_buf()->IsEmpty();
  if (connection_->write_buf()->Append(data) && !writing_in_progress)
    DoWriteLoop();
}

void TCPClient::Close() {
  if (connection_ == nullptr)
    return;

  cr::ThreadTaskRunnerHandle::Get()->DeleteSoon(
      CR_FROM_HERE,
      std::move(connection_));

  delegate_->OnClose();
}

void TCPClient::SetReceiveBufferSize(int32_t size) {
  if (connection_ == nullptr)
    return;

  connection_->read_buf()->set_max_buffer_size(size);
}

void TCPClient::SetSendBufferSize(int32_t size) {
  if (connection_ == nullptr)
    return;

  connection_->write_buf()->set_max_buffer_size(size);
}

void TCPClient::DoConnect() {
  int rv = socket_->Connect(
      cr::BindOnce(&TCPClient::HandleConnectResult, 
                   weak_ptr_factory_.GetWeakPtr()));
  if (rv == ERR_IO_PENDING) 
    return;
  
  HandleConnectResult(rv);
}

void TCPClient::HandleConnectResult(int rv) {
  if (rv < 0) {
    CR_LOG(Error) << "Connect error: " << crnet::ErrorToString(rv);
    socket_.reset();
    delegate_->OnConnect(rv);
    return;
  }

  connection_ = std::make_unique<TCPConnection>(0, std::move(socket_));
  delegate_->OnConnect(rv);

  if (HasClosedConnection()) 
    return;

  DoReadLoop();
}

void TCPClient::DoReadLoop() {
  int rv;
  do {
    cr::ReadIOBuffer* read_buf = connection_->read_buf();

    // Increases read buffer size if necessary.
    if (read_buf->RemainingCapacity() == 0 && !read_buf->IncreaseCapacity()) {
      Close();
      return;
    }

    rv = connection_->socket()->Read(
        read_buf, read_buf->RemainingCapacity(),
        cr::BindOnce(&TCPClient::OnReadCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
    if (rv == ERR_IO_PENDING)
      return;
    rv = HandleReadResult(rv);
  } while (rv == OK);
}

void TCPClient::OnReadCompleted(int rv) {
  if (connection_ == nullptr)
    return; // closed..

  if (HandleReadResult(rv) == OK)
    DoReadLoop();
}

int TCPClient::HandleReadResult(int rv) {
  if (rv <= 0) {
    Close();
    return rv == 0 ? ERR_CONNECTION_CLOSED : rv;
  }

  cr::ReadIOBuffer* read_buf = connection_->read_buf();
  read_buf->DidRead(rv);

  // Handles stream data.
  while (!read_buf->readable_bytes().empty()) {
    int handled = delegate_->OnReceiveData(
      reinterpret_cast<const char*>(read_buf->readable_bytes().data()),
      static_cast<int>(read_buf->readable_bytes().size()));
    if (handled == 0) {
      break;
    }
    else if (handled < 0) {
      // An error has been occured. Closing the connection.
      Close();
      return ERR_CONNECTION_CLOSED;
    }

    read_buf->DidConsume(handled);

    if (HasClosedConnection())
      return ERR_CONNECTION_CLOSED;
  }

  return OK;
}

void TCPClient::DoWriteLoop() {
  int rv = OK;
  cr::QueuedWriteIOBuffer* write_buf = connection_->write_buf();
  while (rv == OK && write_buf->GetSizeToWrite() > 0) {
    rv = connection_->socket()->Write(
        write_buf, write_buf->GetSizeToWrite(),
        cr::BindOnce(&TCPClient::OnWriteCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
    if (rv == ERR_IO_PENDING || rv == OK)
      return;
    rv = HandleWriteResult(rv);
  }
}

void TCPClient::OnWriteCompleted(int rv) {
  if (connection_ == nullptr)
    return; // closed..

  if (HandleWriteResult(rv) == OK)
    DoWriteLoop();
}

int TCPClient::HandleWriteResult(int rv) {
  if (rv < 0) {
    Close();
    return rv;
  }

  connection_->write_buf()->DidConsume(rv);
  return OK;
}

bool TCPClient::HasClosedConnection() {
  return connection_ == nullptr;
}

}  // namespace crnet
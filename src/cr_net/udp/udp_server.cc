// Copyright 2004 The WebRTC Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "cr_net/udp/udp_server.h"

#include "cr_base/buffer/byte_buffer.h"
#include "cr_base/memory/ref_ptr.h"
#include "cr_base/logging/logging.h"
#include "cr_base/location.h"
#include "cr_base/functional/bind.h"
#include "cr_event/single_thread_task_runner.h"
#include "cr_event/threading/thread_task_runner_handle.h"

#include "cr_net/base/net_errors.h"

namespace crnet {

UDPServer::UDPServer(std::unique_ptr<DatagramServerSocket> socket,
                     Delegate* delegate)
    : socket_(std::move(socket)),
      delegate_(delegate),
      read_buf_(cr::MakeRefCounted<cr::ReadIOBuffer>()),
      write_queue_(cr::MakeRefCounted<cr::QueuedWriteIOBuffer>()) {
  CR_DCHECK(socket_);

  // Start accepting connections in next run loop in case when delegate is not
  // ready to get callbacks.
  cr::ThreadTaskRunnerHandle::Get()->PostTask(
      CR_FROM_HERE, cr::BindOnce(&UDPServer::DoRecvFromLoop,
                                 weak_ptr_factory_.GetWeakPtr()));
}

UDPServer::~UDPServer() {
}

void UDPServer::SendData(const IPEndPoint& end_point, 
                         cr::Span<const char> data) {
  bool writing_in_progress = !write_queue_->IsEmpty();
  if (!write_queue_->Append(data))
    return ;
  write_queue_ep_.push(end_point);

  if (!writing_in_progress)
    DoWriteLoop();
}

void UDPServer::SetReceiveBufferSize(int32_t size) {
  read_buf_->SetCapacity(size);
  read_buf_->set_max_buffer_size(size);
}

void UDPServer::SetSendBufferSize(int32_t size) {
  write_queue_->set_max_buffer_size(size);
}

// - read --

void UDPServer::DoRecvFromLoop() {
  int rv;

  do {
    rv = socket_->RecvFrom(
        read_buf_.get(), 
        read_buf_->RemainingCapacity(), 
        &end_point_,
        cr::BindOnce(&UDPServer::OnRecvFromCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
    if (rv == ERR_IO_PENDING) {
      return ;
    }

    rv = HandleRecvFromResult(rv);
  } while (rv == OK);
}

void UDPServer::OnRecvFromCompleted(int rv) {
  if (HandleRecvFromResult(rv) == OK)
    DoRecvFromLoop();
}

int UDPServer::HandleRecvFromResult(int rv) {
  if (rv < 0) {
    CR_LOG(Error) << "RecvFrom error: rv=" << rv;
    return rv;
  }

  HandleReadedData(end_point_, read_buf_->data(), rv);
  return OK;
}

// - write --

void UDPServer::DoWriteLoop() {
  CR_DCHECK(write_queue_ep_.size() == write_queue_->queue_size());

  int rv = OK;
  cr::QueuedWriteIOBuffer* write_buf = write_queue_.get();
  while (rv == OK && write_buf->GetSizeToWrite() > 0) {
    rv = socket_->SendTo(
        write_buf, 
        write_buf->GetSizeToWrite(), 
        write_queue_ep_.front(),
        cr::BindOnce(&UDPServer::OnWriteCompleted,
                      weak_ptr_factory_.GetWeakPtr()));
    if (rv == ERR_IO_PENDING || rv == OK)
      return; // waiting for OnWriteCompleted.

    rv = HandleWriteResult(rv);
  }
}

void UDPServer::OnWriteCompleted(int rv) {
  if (HandleWriteResult(rv) == OK)
    DoWriteLoop();
}

int UDPServer::HandleWriteResult(int rv) {
  // always discard the head data.
  if (!write_queue_ep_.empty())
    write_queue_ep_.pop();
  write_queue_->DidConsume(write_queue_->GetSizeToWrite());

  CR_DCHECK(write_queue_ep_.size() == write_queue_->queue_size());
  if (rv < 0) {
    return rv;
  }
  return OK;
}

void UDPServer::HandleReadedData(const IPEndPoint& end_point,
                                 const char* data,
                                 int size) {
  if (delegate_)
    delegate_->OnReceiveData(end_point, data, size);
}

}  // namespace crnet
// Copyright 2004 The WebRTC Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "cr_net/udp/udp_client.h"

#include "cr_base/buffer/byte_buffer.h"
#include "cr_base/memory/ref_ptr.h"
#include "cr_base/logging/logging.h"
#include "cr_base/location.h"
#include "cr_base/functional/bind.h"
#include "cr_event/single_thread_task_runner.h"
#include "cr_event/threading/thread_task_runner_handle.h"

#include "cr_net/base/net_errors.h"

namespace crnet {

UDPClient::UDPClient(std::unique_ptr<DatagramClientSocket> socket,
                     Delegate* delegate)
    : socket_(std::move(socket)),
      delegate_(delegate),
      read_buf_(cr::MakeRefCounted<cr::ReadIOBuffer>()),
      write_queue_(cr::MakeRefCounted<cr::QueuedWriteIOBuffer>()) {
  CR_DCHECK(socket_);

  // Start accepting connections in next run loop in case when delegate is not
  // ready to get callbacks.
  cr::ThreadTaskRunnerHandle::Get()->PostTask(
      CR_FROM_HERE, cr::BindOnce(&UDPClient::DoReadLoop,
                                 weak_ptr_factory_.GetWeakPtr()));
}

UDPClient::~UDPClient() {
}

void UDPClient::SendData(cr::Span<const char> data) {
  bool writing_in_progress = !write_queue_->IsEmpty();
  if (!write_queue_->Append(data))
    return ;

  if (!writing_in_progress)
    DoWriteLoop();
}

void UDPClient::SetReceiveBufferSize(int32_t size) {
  read_buf_->set_max_buffer_size(size);
}

void UDPClient::SetSendBufferSize(int32_t size) {
  write_queue_->set_max_buffer_size(size);
}

// - read --

void UDPClient::DoReadLoop() {
  int rv;

  do {
    rv = socket_->Read(
        read_buf_.get(), 
        read_buf_->RemainingCapacity(), 
        cr::BindOnce(&UDPClient::OnReadCompleted,
                     weak_ptr_factory_.GetWeakPtr()));
    if (rv == ERR_IO_PENDING) {
      return ;
    }

    rv = HandleReadResult(rv);
  } while (rv == OK);
}

void UDPClient::OnReadCompleted(int rv) {
  if (HandleReadResult(rv) == OK)
    DoReadLoop();
}

int UDPClient::HandleReadResult(int rv) {
  if (rv < 0) {
    CR_LOG(Error) << "Read error: rv=" << rv;
    return rv;
  }

  HandleReadedData(read_buf_->data(), rv);
  return OK;
}

// - write --

void UDPClient::DoWriteLoop() {
  int rv = OK;
  cr::QueuedWriteIOBuffer* write_buf = write_queue_.get();
  while (rv == OK && write_buf->GetSizeToWrite() > 0) {
    rv = socket_->Write(
        write_buf, 
        write_buf->GetSizeToWrite(), 
        cr::BindOnce(&UDPClient::OnWriteCompleted,
                      weak_ptr_factory_.GetWeakPtr()));
    if (rv == ERR_IO_PENDING || rv == OK)
      return; // waiting for OnWriteCompleted.

    rv = HandleWriteResult(rv);
  }
}

void UDPClient::OnWriteCompleted(int rv) {
  if (HandleWriteResult(rv) == OK)
    DoWriteLoop();
}

int UDPClient::HandleWriteResult(int rv) {
  // always discard the head data.
  write_queue_->DidConsume(write_queue_->GetSizeToWrite());
  if (rv < 0) {
    return rv;
  }
  return OK;
}

void UDPClient::HandleReadedData(const char* data,
                                 int size) {
  if (delegate_)
    delegate_->OnReceiveData(data, size);
}

}  // namespace crnet
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/tcp/tcp_connection.h"

#include <utility>

#include "crbase/logging.h"
#include "crnet/socket/tcp/stream_socket.h"

namespace crnet {

TCPConnection::ReadIOBuffer::ReadIOBuffer()
    : base_(cr::MakeRefCounted<cr::GrowableIOBuffer>()),
      max_buffer_size_(kDefaultMaxBufferSize) {
  SetCapacity(kInitialBufSize);
}

TCPConnection::ReadIOBuffer::~ReadIOBuffer() {
  // Avoid dangling ptr when `base_` is destroyed.
  ClearSpan();
}

int TCPConnection::ReadIOBuffer::GetCapacity() const {
  return base_->capacity();
}

void TCPConnection::ReadIOBuffer::SetCapacity(int capacity) {
  CR_DCHECK(base_->offset() <= capacity);
  // Clear current span to avoid warning about dangling pointer, as
  // SetCapacity() may destroy the old buffer.
  ClearSpan();
  base_->SetCapacity(capacity);
  SetSpan(base_->span());
}

bool TCPConnection::ReadIOBuffer::IncreaseCapacity() {
  if (GetCapacity() >= max_buffer_size_) {
    CR_LOG(Error) 
        << "Too large read data is pending: capacity=" << GetCapacity()
        << ", max_buffer_size=" << max_buffer_size_
        << ", read=" << base_->offset();
    return false;
  }

  int new_capacity = GetCapacity() * kCapacityIncreaseFactor;
  if (new_capacity > max_buffer_size_)
    new_capacity = max_buffer_size_;
  SetCapacity(new_capacity);
  return true;
}

cr::Span<const uint8_t> TCPConnection::ReadIOBuffer::readable_bytes() const {
  return base_->span_before_offset();
}

void TCPConnection::ReadIOBuffer::DidRead(int bytes) {
  CR_DCHECK(RemainingCapacity() >= bytes);
  base_->set_offset(base_->offset() + bytes);
  SetSpan(base_->span());
}

int TCPConnection::ReadIOBuffer::RemainingCapacity() const {
  return base_->RemainingCapacity();
}

void TCPConnection::ReadIOBuffer::DidConsume(int bytes) {
  int previous_size = base_->offset();
  int unconsumed_size = previous_size - bytes;
  CR_DCHECK(0 <= unconsumed_size);
  if (unconsumed_size > 0) {
    // Move unconsumed data to the start of buffer. readable_bytes() returns a
    // read-only buffer, so need to call the non-constant overload in the base
    // class instead, to get a writeable span.
    cr::Span<uint8_t> buffer = base_->span_before_offset();
    memmove(buffer.begin(), buffer.subview(bytes).data(), unconsumed_size);
  }
  base_->set_offset(unconsumed_size);
  SetSpan(base_->span());

  // If capacity is too big, reduce it.
  if (GetCapacity() > kMinimumBufSize &&
      GetCapacity() > previous_size * kCapacityIncreaseFactor) {
    int new_capacity = GetCapacity() / kCapacityIncreaseFactor;
    if (new_capacity < kMinimumBufSize)
      new_capacity = kMinimumBufSize;
    // this avoids the pointer to dangle until `SetCapacity` gets called.
    ClearSpan();
    // realloc() within GrowableIOBuffer::SetCapacity() could move data even
    // when size is reduced. If unconsumed_size == 0, i.e. no data exists in
    // the buffer, free internal buffer first to guarantee no data move.
    if (!unconsumed_size)
      base_->SetCapacity(0);
    SetCapacity(new_capacity);
  }
}

TCPConnection::QueuedWriteIOBuffer::QueuedWriteIOBuffer()
    : total_size_(0),
      max_buffer_size_(kDefaultMaxBufferSize) {
}

TCPConnection::QueuedWriteIOBuffer::~QueuedWriteIOBuffer() {
  // `pending_data_` owns the underlying data.
  ClearSpan();
}

bool TCPConnection::QueuedWriteIOBuffer::IsEmpty() const {
  return pending_data_.empty();
}

bool TCPConnection::QueuedWriteIOBuffer::Append(const std::string& data) {
  if (data.empty())
    return true;

  if (total_size_ + static_cast<int>(data.size()) > max_buffer_size_) {
    CR_LOG(Error) << "Too large write data is pending: size="
                  << total_size_ + data.size()
                  << ", max_buffer_size=" << max_buffer_size_;
    return false;
  }

  pending_data_.push(std::make_unique<std::string>(data));
  total_size_ += static_cast<int>(data.size());

  // If new data is the first pending data, updates data_.
  if (pending_data_.size() == 1) {
    SetSpan(cr::make_bytes_span(*pending_data_.front()));
  }
  return true;
}

void TCPConnection::QueuedWriteIOBuffer::DidConsume(int size) {
  CR_DCHECK(total_size_ >= size);
  CR_DCHECK(GetSizeToWrite() >= size);
  if (size == 0)
    return;

  if (size < GetSizeToWrite()) {
    SetSpan(span().subview(cr::checked_cast<size_t>(size)));
  } else {  
    // size == GetSizeToWrite(). Updates data_ to next pending data.
    ClearSpan();
    pending_data_.pop();
    if (!IsEmpty()) {
      SetSpan(cr::make_bytes_span(*pending_data_.front()));
    }
  }
  total_size_ -= size;
}

int TCPConnection::QueuedWriteIOBuffer::GetSizeToWrite() const {
  if (IsEmpty()) {
    CR_DCHECK(0 == total_size_);
    return 0;
  }
  return size();
}

TCPConnection::TCPConnection(TCPConnection::ID id, 
                             std::unique_ptr<StreamSocket> socket)
    : id_(id),
      socket_(std::move(socket)),
      read_buf_(cr::MakeRefCounted<ReadIOBuffer>()),
      write_buf_(cr::MakeRefCounted<QueuedWriteIOBuffer>()) {}

TCPConnection::~TCPConnection() = default;

}  // namespace crnet
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/ipc_channel_reader.h"

#include <stddef.h>

#include <algorithm>

#include "crbase/logging.h"
#include "crbase/message_loop/message_loop.h"
#include "cripc/ipc_delegate.h"

namespace cripc {
namespace internal {

ChannelReader::ChannelReader(Channel::Delegate* delegate)
  : delegate_(delegate),
    max_input_buffer_size_(Channel::kMaximumReadBufferSize),
    input_buffer_handled_size_(0) {
  CR_DCHECK(delegate_ != nullptr);
  memset(input_buf_, 0, sizeof(input_buf_));
}

ChannelReader::~ChannelReader() {
}

ChannelReader::DispatchState ChannelReader::ProcessIncomingData() {
  while (true) {
    int bytes_read = 0;
    ReadState read_state = ReadData(input_buf_, Channel::kReadBufferSize,
                                    &bytes_read);
    if (read_state == READ_FAILED)
      return DISPATCH_ERROR;
    if (read_state == READ_PENDING)
      return DISPATCH_FINISHED;

    CR_DCHECK(bytes_read > 0);
    if (!TranslateInputData(input_buf_, bytes_read))
      return DISPATCH_ERROR;
    return DISPATCH_FINISHED;
  }
}

ChannelReader::DispatchState ChannelReader::AsyncReadComplete(int bytes_read) {
  CR_DCHECK(bytes_read > 0);
  if (!TranslateInputData(input_buf_, bytes_read))
    return DISPATCH_ERROR;

  return DISPATCH_FINISHED;
}

bool ChannelReader::TranslateInputData(const char* input_data,
                                       int input_data_len) {
  const char* p;
  const char* end;

  int unhandled_size = 0;

  // Possibly combine with the overflow buffer to make a larger buffer.
  if (input_overflow_buf_.empty()) {
    p = input_data;
    end = input_data + input_data_len;
    unhandled_size = input_data_len;
  } else {
    if (!CheckMessageSize(input_overflow_buf_.size() + input_data_len))
      return false;
    input_overflow_buf_.append(input_data, input_data_len);
    p = input_overflow_buf_.data();
    end = p + input_overflow_buf_.size();
    unhandled_size = static_cast<int>(input_overflow_buf_.size());
  }

  int handled_size = delegate_->OnChannelTranslateData(p, unhandled_size);
  if (handled_size < 0)
    return false;

  if (handled_size > 0)
    OnTranslatedMessage();

  p += handled_size;
  CR_DCHECK(p <= end);

  size_t next_message_size = end - p;

  // Account for the case where last message's byte is in the next data chunk.
  size_t next_message_buffer_size = next_message_size ?
      next_message_size + Channel::kReadBufferSize - 1:
      0;

  // Save any partial data in the overflow buffer.
  if (p != input_overflow_buf_.data())
    input_overflow_buf_.assign(p, end - p);

  if (!input_overflow_buf_.empty()) {
    // We have something in the overflow buffer, which means that we will
    // append the next data chunk (instead of parsing it directly). So we
    // resize the buffer to fit the next message, to avoid repeatedly
    // growing the buffer as we receive all message' data chunks.
    if (next_message_buffer_size > input_overflow_buf_.capacity()) {
      input_overflow_buf_.reserve(next_message_buffer_size);
    }
  }

  // Trim the buffer if we can
  if (next_message_buffer_size < max_input_buffer_size_ &&
      input_overflow_buf_.size() < max_input_buffer_size_ &&
      input_overflow_buf_.capacity() > max_input_buffer_size_) {
    // std::string doesn't really have a method to shrink capacity to
    // a specific value, so we have to swap with another string.
    std::string trimmed_buf;
    trimmed_buf.reserve(max_input_buffer_size_);
    if (trimmed_buf.capacity() > max_input_buffer_size_) {
      // Since we don't control how much space reserve() actually reserves,
      // we have to go other way around and change the max size to avoid
      // getting into the outer if() again.
      max_input_buffer_size_ = trimmed_buf.capacity();
    }
    trimmed_buf.assign(input_overflow_buf_.data(),
                       input_overflow_buf_.size());
    input_overflow_buf_.swap(trimmed_buf);
  }

  if (input_overflow_buf_.empty() && !DidEmptyInputBuffers())
    return false;
  return true;
}

bool ChannelReader::CheckMessageSize(size_t size) {
  if (size <= Channel::kMaximumMessageSize) {
    return true;
  }
  input_overflow_buf_.clear();
  CR_LOG(Error) << "cripc message is too big: " << size;
  return false;
}

}  // namespace internal
}  // namespace cripc
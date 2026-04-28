// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/base/upload_data_stream.h"

#include "cr_base/logging/logging.h"
#include "cr_event/io_buffer.h"
#include "cr_net/base/net_errors.h"
///#include "cr_net/log/net_log_event_type.h"

namespace crnet {

UploadDataStream::UploadDataStream(bool is_chunked, int64_t identifier)
    : total_size_(0),
      current_position_(0),
      identifier_(identifier),
      is_chunked_(is_chunked),
      initialized_successfully_(false),
      is_eof_(false) {
}

UploadDataStream::~UploadDataStream() = default;

int UploadDataStream::Init(CompletionOnceCallback callback/*,
                           const NetLogWithSource& net_log*/) {
  Reset();
  CR_DCHECK(!initialized_successfully_);
  CR_DCHECK(callback_.is_null());
  CR_DCHECK(!callback.is_null() || IsInMemory());
  ///net_log_ = net_log;
  ///net_log_.BeginEvent(NetLogEventType::UPLOAD_DATA_STREAM_INIT);

  int result = InitInternal(/*net_log_*/);
  if (result == ERR_IO_PENDING) {
    CR_DCHECK(!IsInMemory());
    callback_ = std::move(callback);
  } else {
    OnInitCompleted(result);
  }

  return result;
}

int UploadDataStream::Read(cr::IOBuffer* buf,
                           int buf_len,
                           CompletionOnceCallback callback) {
  CR_DCHECK(!callback.is_null() || IsInMemory());
  CR_DCHECK(initialized_successfully_);
  CR_DCHECK(buf_len > 0);

  ///net_log_.BeginEvent(NetLogEventType::UPLOAD_DATA_STREAM_READ,
  ///                    [&] { return CreateReadInfoParams(current_position_); });

  int result = 0;
  if (!is_eof_)
    result = ReadInternal(buf, buf_len);

  if (result == ERR_IO_PENDING) {
    CR_DCHECK(!IsInMemory());
    callback_ = std::move(callback);
  } else {
    OnReadCompleted(result);
  }

  return result;
}

bool UploadDataStream::IsEOF() const {
  CR_DCHECK(initialized_successfully_);
  CR_DCHECK(is_chunked_ || is_eof_ == (current_position_ == total_size_));
  return is_eof_;
}

void UploadDataStream::Reset() {
  // If there's a pending callback, there's a pending init or read call that is
  // being canceled.
  ///if (!callback_.is_null()) {
  ///  if (!initialized_successfully_) {
  ///    // If initialization has not yet succeeded, this call is aborting
  ///    // initialization.
  ///    net_log_.EndEventWithNetErrorCode(
  ///        NetLogEventType::UPLOAD_DATA_STREAM_INIT, ERR_ABORTED);
  ///  } else {
  ///    // Otherwise, a read is being aborted.
  ///    net_log_.EndEventWithNetErrorCode(
  ///        NetLogEventType::UPLOAD_DATA_STREAM_READ, ERR_ABORTED);
  ///  }
  ///}

  current_position_ = 0;
  initialized_successfully_ = false;
  is_eof_ = false;
  total_size_ = 0;
  callback_.Reset();
  ResetInternal();
}

void UploadDataStream::SetSize(uint64_t size) {
  CR_DCHECK(!initialized_successfully_);
  CR_DCHECK(!is_chunked_);
  total_size_ = size;
}

void UploadDataStream::SetIsFinalChunk() {
  CR_DCHECK(initialized_successfully_);
  CR_DCHECK(is_chunked_);
  CR_DCHECK(!is_eof_);
  is_eof_ = true;
}

bool UploadDataStream::IsInMemory() const {
  return false;
}

const std::vector<std::unique_ptr<UploadElementReader>>*
UploadDataStream::GetElementReaders() const {
  return nullptr;
}

void UploadDataStream::OnInitCompleted(int result) {
  CR_DCHECK(ERR_IO_PENDING != result);
  CR_DCHECK(!initialized_successfully_);
  CR_DCHECK(0u == current_position_);
  CR_DCHECK(!is_eof_);

  if (result == OK) {
    initialized_successfully_ = true;
    if (!is_chunked_ && total_size_ == 0)
      is_eof_ = true;
  }

  ///net_log_.EndEvent(NetLogEventType::UPLOAD_DATA_STREAM_INIT, [&] {
  ///  return NetLogInitEndInfoParams(result, total_size_, is_chunked_);
  ///});

  if (!callback_.is_null())
    std::move(callback_).Run(result);
}

void UploadDataStream::OnReadCompleted(int result) {
  CR_DCHECK(initialized_successfully_);
  CR_DCHECK(result != 0 || is_eof_);
  CR_DCHECK(ERR_IO_PENDING != result);

  if (result > 0) {
    current_position_ += result;
    if (!is_chunked_) {
      CR_DCHECK(current_position_ <= total_size_);
      if (current_position_ == total_size_)
        is_eof_ = true;
    }
  }

  ///net_log_.EndEventWithNetErrorCode(NetLogEventType::UPLOAD_DATA_STREAM_READ,
  ///                                  result);

  if (!callback_.is_null())
    std::move(callback_).Run(result);
}

UploadProgress UploadDataStream::GetUploadProgress() const {
  // While initialization / rewinding is in progress, return nothing.
  if (!initialized_successfully_)
    return UploadProgress();

  return UploadProgress(current_position_, total_size_);
}

bool UploadDataStream::AllowHTTP1() const {
  return true;
}

}  // namespace crnet

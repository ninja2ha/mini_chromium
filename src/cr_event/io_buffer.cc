// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_event/io_buffer.h"

#include "cr_base/numerics/safe_math.h"

namespace cr {

// --- IOBuffer ---

// TODO(eroman): IOBuffer is being converted to require buffer sizes and offsets
// be specified as "size_t" rather than "int" (crbug.com/488553). To facilitate
// this move (since LOTS of code needs to be updated), this function ensures
// that sizes can be safely converted to an "int" without truncation. The
// assert ensures calling this with an "int" argument is also safe.
// static 
void IOBuffer::AssertValidBufferSize(size_t size) {
  static_assert(sizeof(size_t) >= sizeof(int), "");
  cr::CheckedNumeric<int>(size).ValueOrDie();
}

IOBuffer::IOBuffer() = default;

IOBuffer::IOBuffer(cr::Span<char> span) : 
  IOBuffer(cr::AsWritableBytes(span)){
}

IOBuffer::IOBuffer(cr::Span<uint8_t> span) : span_(span){
  AssertValidBufferSize(span_.size());
}

IOBuffer::~IOBuffer() = default;

void IOBuffer::SetSpan(cr::Span<uint8_t> span) {
  AssertValidBufferSize(span.size());
  span_ = span;
}

void IOBuffer::ClearSpan() {
  span_ = cr::Span<uint8_t>();
}

// ---

StringIOBuffer::StringIOBuffer(std::string s) : string_data_(std::move(s)) {
  // Can't pass `s.data()` directly to IOBuffer constructor since moving
  // from `s` may invalidate it. This is especially true for libc++ short
  // string optimization where the data may be held in the string variable
  // itself, instead of in a movable backing store.
  SetSpan(cr::AsWritableBytes(cr::Span<char>(string_data_)));
}

StringIOBuffer::StringIOBuffer(Span<const char> s) {
  AssertValidBufferSize(s.size());

  string_data_.assign(s.data(), s.size());
  SetSpan(cr::AsWritableBytes(cr::Span<char>(string_data_)));
}

StringIOBuffer::StringIOBuffer(std::unique_ptr<std::string> s) {
  string_data_.swap(*s.get());
  SetSpan(cr::AsWritableBytes(cr::Span<char>(string_data_)));
}

StringIOBuffer::StringIOBuffer(size_t s) {
  string_data_.resize(s);
  SetSpan(cr::AsWritableBytes(cr::Span<char>(string_data_)));
}

StringIOBuffer::~StringIOBuffer() {
  ClearSpan();
}

// --- GrowableIOBuffer  ---

GrowableIOBuffer::GrowableIOBuffer() = default;

void GrowableIOBuffer::SetCapacity(int capacity) {
  CR_CHECK(capacity >= 0);

  // The span will be set again in `set_offset()`. Need to clear raw pointers to
  // the data before reallocating the buffer.
  ClearSpan();

  // realloc will crash if it fails.
  real_data_.reset(
      static_cast<uint8_t*>(realloc(real_data_.release(), capacity)));

  capacity_ = capacity;
  if (offset_ > capacity)
    set_offset(capacity);
  else
    set_offset(offset_);  // The pointer may have changed.
}

void GrowableIOBuffer::set_offset(int offset) {
  CR_CHECK(offset >= 0);
  CR_CHECK(offset <= capacity_);
  offset_ = offset;

  SetSpan(Span<uint8_t>(
      real_data_.get() + offset, static_cast<size_t>(capacity_ - offset)));
}

void GrowableIOBuffer::DidConsume(int bytes) {
  CR_CHECK(0 <= bytes);
  CR_CHECK(bytes <= size());
  set_offset(offset_ + bytes);
}

int GrowableIOBuffer::RemainingCapacity() {
  return size();
}

Span<uint8_t> GrowableIOBuffer::everything() {
  return 
      cr::MakeSpan(
          real_data_.get(), cr::checked_cast<size_t>(capacity_));
}

Span<const uint8_t> GrowableIOBuffer::everything() const {
  return 
      cr::MakeSpan(
          real_data_.get(), cr::checked_cast<size_t>(capacity_));
}

Span<uint8_t> GrowableIOBuffer::span_before_offset() {
  return everything().first(cr::checked_cast<size_t>(offset_));
}

Span<const uint8_t> GrowableIOBuffer::span_before_offset() const {
  return everything().first(cr::checked_cast<size_t>(offset_));
}

GrowableIOBuffer::~GrowableIOBuffer() {
  ClearSpan();
}

// --- ReadIOBuffer --

ReadIOBuffer::ReadIOBuffer()
    : base_(cr::MakeRefCounted<cr::GrowableIOBuffer>()),
      max_buffer_size_(kDefaultMaxBufferSize) {
  SetCapacity(kInitialBufSize);
}

ReadIOBuffer::~ReadIOBuffer() {
  // Avoid dangling ptr when `base_` is destroyed.
  ClearSpan();
}

int ReadIOBuffer::GetCapacity() const {
  return base_->capacity();
}

void ReadIOBuffer::SetCapacity(int capacity) {
  CR_DCHECK(base_->offset() <= capacity);
  // Clear current span to avoid warning about dangling pointer, as
  // SetCapacity() may destroy the old buffer.
  ClearSpan();
  base_->SetCapacity(capacity);
  SetSpan(base_->span());
}

bool ReadIOBuffer::IncreaseCapacity() {
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

cr::Span<const uint8_t> ReadIOBuffer::readable_bytes() const {
  return base_->span_before_offset();
}

void ReadIOBuffer::DidRead(int bytes) {
  CR_DCHECK(RemainingCapacity() >= bytes);
  base_->set_offset(base_->offset() + bytes);
  SetSpan(base_->span());
}

int ReadIOBuffer::RemainingCapacity() const {
  return base_->RemainingCapacity();
}

void ReadIOBuffer::DidConsume(int bytes) {
  int previous_size = base_->offset();
  int unconsumed_size = previous_size - bytes;
  CR_DCHECK(0 <= unconsumed_size);
  if (unconsumed_size > 0) {
    // Move unconsumed data to the start of buffer. readable_bytes() returns a
    // read-only buffer, so need to call the non-constant overload in the base
    // class instead, to get a writeable span.
    cr::Span<uint8_t> buffer = base_->span_before_offset();
    memmove(buffer.data(), buffer.subspan(bytes).data(), unconsumed_size);
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

// --- QueuedWriteIOBuffer --

QueuedWriteIOBuffer::QueuedWriteIOBuffer()
    : total_size_(0),
      max_buffer_size_(kDefaultMaxBufferSize) {
}

QueuedWriteIOBuffer::~QueuedWriteIOBuffer() {
  // `pending_data_` owns the underlying data.
  ClearSpan();
}

bool QueuedWriteIOBuffer::IsEmpty() const {
  return pending_data_.empty();
}

bool QueuedWriteIOBuffer::Append(Span<const char>  data) {
  if (data.empty())
    return true;

  if (total_size_ + static_cast<int>(data.size()) > max_buffer_size_) {
    CR_LOG(Error) << "Too large write data is pending: size="
                  << total_size_ + data.size()
                  << ", max_buffer_size=" << max_buffer_size_;
    return false;
  }

  pending_data_.push(
      std::make_unique<std::string>(data.data(), data.size()));
  total_size_ += static_cast<int>(data.size());

  // If new data is the first pending data, updates data_.
  if (pending_data_.size() == 1) {
    SetSpan(cr::AsWritableBytes(cr::Span<char>(*pending_data_.front())));
  }
  return true;
}

void QueuedWriteIOBuffer::DidConsume(int size) {
  CR_DCHECK(total_size_ >= size);
  CR_DCHECK(GetSizeToWrite() >= size);
  if (size == 0)
    return;

  if (size < GetSizeToWrite()) {
    SetSpan(span().subspan(cr::checked_cast<size_t>(size)));
  } else {  
    // size == GetSizeToWrite(). Updates data_ to next pending data.
    ClearSpan();
    pending_data_.pop();
    if (!IsEmpty()) {
      SetSpan(cr::AsWritableBytes(cr::Span<char>(*pending_data_.front())));
    }
  }
  total_size_ -= size;
}

int QueuedWriteIOBuffer::GetSizeToWrite() const {
  if (IsEmpty()) {
    CR_DCHECK(0 == total_size_);
    return 0;
  }
  return size();
}

// --- PickledIOBuffer  ---

///PickledIOBuffer::PickledIOBuffer(std::unique_ptr<const cr::Pickle> pickle) 
///    : IOBuffer(cr::make_bytes_span(pickle->as_bytes(),
///                                   pickle->size())),
///      pickle_(std::move(pickle)) {
///}

///PickledIOBuffer::~PickledIOBuffer() {
///  // Avoid dangling ptr when this destructor destroys the pickle.
///  ClearSpan();
///}

}  // namespace cr
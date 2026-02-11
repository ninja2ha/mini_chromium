// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase_runtime/io_buffer.h"

#include "crbase/numerics/safe_math.h"

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
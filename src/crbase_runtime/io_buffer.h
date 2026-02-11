// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_IO_BUFFER_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_IO_BUFFER_H_

#include <memory>

#include "crbase/containers/span.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/memory/ptr_util.h"
///#include "crbase/buffer/pickle.h"
#include "crbase_runtime/runtime_export.h"

namespace cr {

// IOBuffers are reference counted data buffers used for easier asynchronous
// IO handling.
//
// They are often used as the destination buffers for Read() operations, or as
// the source buffers for Write() operations.
//
// IMPORTANT: Never re-use an IOBuffer after cancelling the IO operation that
//            was using it, since this may lead to memory corruption!
//
// -----------------------
// Ownership of IOBuffers:
// -----------------------
//
// Although IOBuffers are RefCountedThreadSafe, they are not intended to be
// used as a shared buffer, nor should they be used simultaneously across
// threads. The fact that they are reference counted is an implementation
// detail for allowing them to outlive cancellation of asynchronous
// operations.
//
// Instead, think of the underlying |char*| buffer contained by the IOBuffer
// as having exactly one owner at a time.
//
// Whenever you call an asynchronous operation that takes an IOBuffer,
// ownership is implicitly transferred to the called function, until the
// operation has completed (at which point it transfers back to the caller).
//
//     ==> The IOBuffer's data should NOT be manipulated, destroyed, or read
//         until the operation has completed.
//
//     ==> Cancellation does NOT count as completion. If an operation using
//         an IOBuffer is cancelled, the caller should release their
//         reference to this IOBuffer at the time of cancellation since
//         they can no longer use it.
//
// For instance, if you were to call a Read() operation on some class which
// takes an IOBuffer, and then delete that class (which generally will
// trigger cancellation), the IOBuffer which had been passed to Read() should
// never be re-used.
//
// This usage contract is assumed by any API which takes an IOBuffer, even
// though it may not be explicitly mentioned in the function's comments.
//
// -----------------------
// Motivation
// -----------------------
//
// The motivation for transferring ownership during cancellation is
// to make it easier to work with un-cancellable operations.
//
// For instance, let's say under the hood your API called out to the
// operating system's synchronous ReadFile() function on a worker thread.
// When cancelling through our asynchronous interface, we have no way of
// actually aborting the in progress ReadFile(). We must let it keep running,
// and hence the buffer it was reading into must remain alive. Using
// reference counting we can add a reference to the IOBuffer and make sure
// it is not destroyed until after the synchronous operation has completed.

// --- IoBuffer ---

// Base class, never instantiated, does not own the buffer.
class CRBASE_RT_EXPORT IOBuffer : public cr::RefCountedThreadSafe<IOBuffer> {
 public:
  // Returns the length from bytes() to the end of the buffer. Many methods that
  // take an IOBuffer also take a size indicated the number of IOBuffer bytes to
  // use from the start of bytes(). That number must be no more than the size()
  // of the passed in IOBuffer.
  int size() const {
    // SetSpan() ensures this fits in an int.
    return static_cast<int>(span_.size());
  }

  uint8_t* bytes() { return span_.data(); }
  const uint8_t* bytes() const { return span_.data(); }

  char* data() { return reinterpret_cast<char*>(bytes()); }
  const char* data() const { return reinterpret_cast<const char*>(bytes()); }

  cr::Span<uint8_t> span() { return span_; }
  cr::Span<const uint8_t> span() const { return span_; }

  // Convenience methods for accessing the buffer as a span.
  cr::Span<uint8_t> first(size_t count) { return span().first(count); }
  cr::Span<const uint8_t> first(size_t count) const { 
    return span().first(count);
  }

 protected:
  friend class cr::RefCountedThreadSafe<IOBuffer>;

  static void AssertValidBufferSize(size_t size);

  IOBuffer();
  explicit IOBuffer(cr::Span<char> span);
  explicit IOBuffer(cr::Span<uint8_t> span);

  virtual ~IOBuffer();

  void SetSpan(cr::Span<uint8_t> span);
  void ClearSpan();

 private:
  cr::Span<uint8_t> span_;
};

// --- GrowableIOBuffer  ---
// This version provides a resizable buffer and a changeable offset. The values
// returned by size() and bytes() are updated whenever the offset of the buffer
// is set, or the buffer's capacity is changed.
//
// GrowableIOBuffer is useful when you read data progressively without
// knowing the total size in advance. GrowableIOBuffer can be used as
// follows:
//
// buf = cr::MakeRefCounted<GrowableIOBuffer>();
// buf->SetCapacity(1024);  // Initial capacity.
//
// while (!some_stream->IsEOF()) {
//   // Double the capacity if the remaining capacity is empty.
//   if (buf->RemainingCapacity() == 0)
//     buf->SetCapacity(buf->capacity() * 2);
//   int bytes_read = some_stream->Read(buf, buf->RemainingCapacity());
//   buf->set_offset(buf->offset() + bytes_read);
// }
//
class CRBASE_RT_EXPORT GrowableIOBuffer : public IOBuffer {
 public:
  GrowableIOBuffer();

  // realloc memory to the specified capacity.
  void SetCapacity(int capacity);
  int capacity() { return capacity_; }

  // `offset` moves the `data_` pointer, allowing "seeking" in the data.
  void set_offset(int offset);
  int offset() { return offset_; }

  // Advances the offset by `bytes`. It's equivalent to `set_offset(offset() +
  // bytes)`, though does not accept negative values, as they likely indicate a
  // bug.
  void DidConsume(int bytes);

  int RemainingCapacity();

  // Returns the entire buffer, including the bytes before the `offset()`.
  //
  // The `span()` method in the base class only gives the part of the buffer
  // after `offset()`.
  Span<uint8_t> everything();
  Span<const uint8_t> everything() const;

  // Return a span before the `offset()`.
  Span<uint8_t> span_before_offset();
  Span<const uint8_t> span_before_offset() const;

 private:
  ~GrowableIOBuffer() override;

  // TODO(329476354): Convert to std::vector, use reserve()+resize() to make
  // exact reallocs, and remove `capacity_`. Possibly with an allocator the
  // default-initializes, if it's important to not initialize the new memory?
  std::unique_ptr<uint8_t, cr::FreeDeleter> real_data_;
  int capacity_ = 0;
  int offset_ = 0;
};

// --- PickledIOBuffer  ---

// This version allows a Pickle to be used as the storage for a write-style
// operation, avoiding an extra data copy. used in cripc while send message.
// using in ipc module.
///class CRBASE_RT_EXPORT PickledIOBuffer : public IOBuffer {
/// public:
///  explicit PickledIOBuffer(std::unique_ptr<const cr::Pickle> pickle);
///
/// private:
///  ~PickledIOBuffer() override;
///
///  const std::unique_ptr<const cr::Pickle> pickle_;
///};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_IO_BUFFER_H_
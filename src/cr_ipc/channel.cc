// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/channel.h"

#include <stddef.h>
#include <string.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/numerics/safe_math.h"
#include "crbase/bits.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/win_util.h"
#endif

namespace mojo {
namespace core {

namespace {


const size_t kReadBufferSize = 4096;
const size_t kMaxUnusedReadBufferCapacity = 4096;

static_assert(alignof(std::max_align_t) >= kChannelMessageAlignment, "");
Channel::AlignedBuffer MakeAlignedBuffer(size_t size) {
  // Generic allocators (such as malloc) return a pointer that is suitably
  // aligned for storing any type of object with a fundamental alignment
  // requirement. Buffers have no additional alignment requirement beyond that.
  void* ptr = malloc(cr::bits::AlignUp(size, 4));//base::AllocNonScannable(size);
  // Even though the allocator is configured in such a way that it crashes
  // rather than return nullptr, ASAN and friends don't know about that. This
  // CR_CHECK() prevents Clusterfuzz from complaining. crbug.com/1180576.
  CR_CHECK(ptr);
  return Channel::AlignedBuffer(static_cast<char*>(ptr));
}

}  // namespace

// Helper class for managing a Channel's read buffer allocations. This maintains
// a single contiguous buffer with the layout:
//
//   [discarded bytes][occupied bytes][unoccupied bytes]
//
// The Reserve() method ensures that a certain capacity of unoccupied bytes are
// available. It does not claim that capacity and only allocates new capacity
// when strictly necessary.
//
// Claim() marks unoccupied bytes as occupied.
//
// Discard() marks occupied bytes as discarded, signifying that their contents
// can be forgotten or overwritten.
//
// Realign() moves occupied bytes to the front of the buffer so that those
// occupied bytes are properly aligned.
//
// The most common Channel behavior in practice should result in very few
// allocations and copies, as memory is claimed and discarded shortly after
// being reserved, and future reservations will immediately reuse discarded
// memory.
class Channel::ReadBuffer {
 public:
  ReadBuffer(const ReadBuffer&) = delete;
  ReadBuffer& operator=(const ReadBuffer&) = delete;

  ReadBuffer() {
    size_ = kReadBufferSize;
    data_ = MakeAlignedBuffer(size_);
  }

  ~ReadBuffer() { CR_DCHECK(data_); }

  const char* occupied_bytes() const {
    return data_.get() + num_discarded_bytes_;
  }

  size_t num_occupied_bytes() const {
    return num_occupied_bytes_ - num_discarded_bytes_;
  }

  // Ensures the ReadBuffer has enough contiguous space allocated to hold
  // |num_bytes| more bytes; returns the address of the first available byte.
  char* Reserve(size_t num_bytes) {
    if (num_occupied_bytes_ + num_bytes > size_) {
      size_ = std::max(size_ * 2, num_occupied_bytes_ + num_bytes);
      AlignedBuffer new_data = MakeAlignedBuffer(size_);
      memcpy(new_data.get(), data_.get(), num_occupied_bytes_);
      data_ = std::move(new_data);
    }

    return data_.get() + num_occupied_bytes_;
  }

  // Marks the first |num_bytes| unoccupied bytes as occupied.
  void Claim(size_t num_bytes) {
    CR_DCHECK(num_occupied_bytes_ + num_bytes <= size_);
    num_occupied_bytes_ += num_bytes;
  }

  // Marks the first |num_bytes| occupied bytes as discarded. This may result in
  // shrinkage of the internal buffer, and it is not safe to assume the result
  // of a previous Reserve() call is still valid after this.
  void Discard(size_t num_bytes) {
    CR_DCHECK((num_discarded_bytes_ + num_bytes) <= num_occupied_bytes_);
    num_discarded_bytes_ += num_bytes;

    if (num_discarded_bytes_ == num_occupied_bytes_) {
      // We can just reuse the buffer from the beginning in this common case.
      num_discarded_bytes_ = 0;
      num_occupied_bytes_ = 0;
    }

    if (num_discarded_bytes_ > kMaxUnusedReadBufferCapacity) {
      // In the uncommon case that we have a lot of discarded data at the
      // front of the buffer, simply move remaining data to a smaller buffer.
      size_t num_preserved_bytes = num_occupied_bytes_ - num_discarded_bytes_;
      size_ = std::max(num_preserved_bytes, kReadBufferSize);
      AlignedBuffer new_data = MakeAlignedBuffer(size_);
      memcpy(new_data.get(), data_.get() + num_discarded_bytes_,
             num_preserved_bytes);
      data_ = std::move(new_data);
      num_discarded_bytes_ = 0;
      num_occupied_bytes_ = num_preserved_bytes;
    }

    if (num_occupied_bytes_ == 0 && size_ > kMaxUnusedReadBufferCapacity) {
      // Opportunistically shrink the read buffer back down to a small size if
      // it's grown very large. We only do this if there are no remaining
      // unconsumed bytes in the buffer to avoid copies in most the common
      // cases.
      size_ = kMaxUnusedReadBufferCapacity;
      data_ = MakeAlignedBuffer(size_);
    }
  }

  void Realign() {
    size_t num_bytes = num_occupied_bytes();
    memmove(data_.get(), occupied_bytes(), num_bytes);
    num_discarded_bytes_ = 0;
    num_occupied_bytes_ = num_bytes;
  }

 private:
  AlignedBuffer data_;

  // The total size of the allocated buffer.
  size_t size_ = 0;

  // The number of discarded bytes at the beginning of the allocated buffer.
  size_t num_discarded_bytes_ = 0;

  // The total number of occupied bytes, including discarded bytes.
  size_t num_occupied_bytes_ = 0;
};

Channel::Channel(Delegate* delegate)
    : delegate_(delegate),
      read_buffer_(nullptr) {}

Channel::~Channel() = default;

void Channel::ShutDown() {
  ShutDownImpl();
  delegate_ = nullptr;
}

char* Channel::GetReadBuffer(size_t* buffer_capacity) {
  CR_DCHECK(read_buffer_);
  size_t required_capacity = *buffer_capacity;
  if (!required_capacity)
    required_capacity = kReadBufferSize;

  *buffer_capacity = required_capacity;
  return read_buffer_->Reserve(required_capacity);
}

bool Channel::OnReadComplete(size_t bytes_read, size_t* next_read_size_hint) {
  CR_DCHECK(read_buffer_);
  *next_read_size_hint = kReadBufferSize;
  read_buffer_->Claim(bytes_read);
  while (read_buffer_->num_occupied_bytes() > 0) {
    // Ensure the occupied data is properly aligned. If it isn't, a SIGBUS could
    // happen on architectures that don't allow misaligned words access (i.e.
    // anything other than x86). Only re-align when necessary to avoid copies.
    if (!IsAlignedForChannelMessage(
            reinterpret_cast<uintptr_t>(read_buffer_->occupied_bytes()))) {
      read_buffer_->Realign();
    }

    DispatchResult result =
        TryDispatchMessage(cr::MakeSpan(read_buffer_->occupied_bytes(),
                                        read_buffer_->num_occupied_bytes()),
                           next_read_size_hint);
    if (result == DispatchResult::kOK) {
      read_buffer_->Discard(*next_read_size_hint);
      *next_read_size_hint = 0;
    } else if (result == DispatchResult::kNotEnoughData) {
      return true;
    } else if (result == DispatchResult::kMissingHandles) {
      break;
    } else if (result == DispatchResult::kError) {
      return false;
    }
  }
  return true;
}

Channel::DispatchResult Channel::TryDispatchMessage(
    cr::Span<const char> buffer,
    size_t* size_hint) {
  *size_hint = buffer.size();
  return DispatchResult::kOK;
}

void Channel::OnError(Error error) {
  if (delegate_)
    delegate_->OnChannelError(error);
}

}  // namespace core
}  // namespace mojo
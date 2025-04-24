// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/buffer/pickle.h"

#include <stdlib.h>
#include <memory.h>

#include <algorithm>  // for max()
#include <limits>

#include "crbase/logging.h"
#include "crbase/bits.h"
#include "crbase/numerics/safe_math.h"
#include "crbase/build_platform.h"

namespace cr {

// static
const int Pickle::kPayloadUnit = 64;

static const size_t kCapacityReadOnly = static_cast<size_t>(-1);

PickleIterator::PickleIterator(const Pickle& pickle)
    : payload_(pickle.payload()),
      read_index_(0),
      end_index_(pickle.payload_size()) {
}

template <typename Type>
inline bool PickleIterator::ReadBuiltinType(Type* result) {
  const char* read_from = GetReadPointerAndAdvance<Type>();
  if (!read_from)
    return false;
  if (sizeof(Type) > sizeof(uint32_t))
    memcpy(result, read_from, sizeof(*result));
  else
    *result = *reinterpret_cast<const Type*>(read_from);
  return true;
}

inline void PickleIterator::Advance(size_t size) {
  size_t aligned_size = bits::AlignUp(size, sizeof(uint32_t));
  if (end_index_ - read_index_ < aligned_size) {
    read_index_ = end_index_;
  } else {
    read_index_ += aligned_size;
  }
}

template<typename Type>
inline const char* PickleIterator::GetReadPointerAndAdvance() {
  if (sizeof(Type) > end_index_ - read_index_) {
    read_index_ = end_index_;
    return NULL;
  }
  const char* current_read_ptr = payload_ + read_index_;
  Advance(sizeof(Type));
  return current_read_ptr;
}

const char* PickleIterator::GetReadPointerAndAdvance(size_t num_bytes) {
  if (num_bytes > end_index_ - read_index_) {
    read_index_ = end_index_;
    return NULL;
  }
  const char* current_read_ptr = payload_ + read_index_;
  Advance(num_bytes);
  return current_read_ptr;
}

inline const char* PickleIterator::GetReadPointerAndAdvance(
    size_t num_elements,
    size_t size_element) {
  // Check for size_t overflow.
  size_t num_bytes;
  if (!CheckMul(num_elements, size_element).AssignIfValid(&num_bytes)) {
    return nullptr;
  }

  return GetReadPointerAndAdvance(num_bytes);
}

bool PickleIterator::ReadBool(bool* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt8(int8_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadUInt8(uint8_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt16(int16_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadUInt16(uint16_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt32(int32_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadUInt32(uint32_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt64(int64_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadUInt64(uint64_t* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt(int* result) {
  return ReadBuiltinType(result);
}

bool PickleIterator::ReadLong(long* result) {
  int64_t r64;
  if (ReadBuiltinType(&r64)) {
    *result = static_cast<long>(r64);
    return true;
  }
  return false;
}

bool PickleIterator::ReadFloat(float* result) {
  // crbug.com/315213
  // The source data may not be properly aligned, and unaligned float reads
  // cause SIGBUS on some ARM platforms, so force using memcpy to copy the data
  // into the result.
  const char* read_from = GetReadPointerAndAdvance<float>();
  if (!read_from)
    return false;
  memcpy(result, read_from, sizeof(*result));
  return true;
}

bool PickleIterator::ReadDouble(double* result) {
  // crbug.com/315213
  // The source data may not be properly aligned, and unaligned double reads
  // cause SIGBUS on some ARM platforms, so force using memcpy to copy the data
  // into the result.
  const char* read_from = GetReadPointerAndAdvance<double>();
  if (!read_from)
    return false;
  memcpy(result, read_from, sizeof(*result));
  return true;
}

bool PickleIterator::ReadString(std::string* result) {
  size_t len;
  if (!ReadLength(&len))
    return false;
  const char* read_from = GetReadPointerAndAdvance(len);
  if (!read_from)
    return false;

  result->assign(read_from, len);
  return true;
}

bool PickleIterator::ReadStringPiece(StringPiece* result) {
  size_t len;
  if (!ReadLength(&len))
    return false;
  const char* read_from = GetReadPointerAndAdvance(len);
  if (!read_from)
    return false;

  *result = StringPiece(read_from, len);
  return true;
}

bool PickleIterator::ReadString16(string16* result) {
  size_t len;
  if (!ReadLength(&len))
    return false;
  const char* read_from = GetReadPointerAndAdvance(len, sizeof(char16));
  if (!read_from)
    return false;

  result->assign(reinterpret_cast<const char16*>(read_from), len);
  return true;
}

bool PickleIterator::ReadStringPiece16(StringPiece16* result) {
  size_t len;
  if (!ReadLength(&len))
    return false;
  const char* read_from = GetReadPointerAndAdvance(len, sizeof(char16));
  if (!read_from)
    return false;

  *result = StringPiece16(reinterpret_cast<const char16*>(read_from), len);
  return true;
}

bool PickleIterator::ReadData(const char** data, size_t* length) {
  *length = 0;
  *data = 0;

  if (!ReadLength(length))
    return false;

  return ReadBytes(data, *length);
}

bool PickleIterator::ReadBytes(const char** data, size_t length) {
  const char* read_from = GetReadPointerAndAdvance(length);
  if (!read_from)
    return false;
  *data = read_from;
  return true;
}

// Payload is uint32_t aligned.

Pickle::Pickle()
    : header_(NULL),
      header_size_(sizeof(Header)),
      capacity_after_header_(0),
      write_offset_(0) {
  static_assert((Pickle::kPayloadUnit & (Pickle::kPayloadUnit - 1)) == 0,
                "Pickle::kPayloadUnit must be a power of two");
  Resize(kPayloadUnit);
  header_->payload_size = 0;
}

Pickle::Pickle(int header_size)
    : header_(NULL),
      header_size_(bits::AlignUp(header_size, sizeof(uint32_t))),
      capacity_after_header_(0),
      write_offset_(0) {
  Resize(kPayloadUnit);
  header_->payload_size = 0;
}

Pickle::Pickle(const char* data, size_t data_len)
    : header_(reinterpret_cast<Header*>(const_cast<char*>(data))),
      header_size_(0),
      capacity_after_header_(kCapacityReadOnly),
      write_offset_(0) {
  if (data_len >= sizeof(Header))
    header_size_ = data_len - header_->payload_size;

  if (header_size_ > static_cast<unsigned int>(data_len))
    header_size_ = 0;

  if (header_size_ != bits::AlignUp(header_size_, sizeof(uint32_t)))
    header_size_ = 0;

  // If there is anything wrong with the data, we're not going to use it.
  if (!header_size_)
    header_ = NULL;
}

Pickle::Pickle(const Pickle& other)
    : header_(NULL),
      header_size_(other.header_size_),
      capacity_after_header_(0),
      write_offset_(other.write_offset_) {
  Resize(other.header_->payload_size);
  memcpy(header_, other.header_, header_size_ + other.header_->payload_size);
}

Pickle::~Pickle() {
  if (capacity_after_header_ != kCapacityReadOnly)
    free(header_);
}

Pickle& Pickle::operator=(const Pickle& other) {
  if (this == &other) {
    return *this;
  }
  if (capacity_after_header_ == kCapacityReadOnly) {
    header_ = NULL;
    capacity_after_header_ = 0;
  }
  if (header_size_ != other.header_size_) {
    free(header_);
    header_ = NULL;
    header_size_ = other.header_size_;
  }
  Resize(other.header_->payload_size);
  memcpy(header_, other.header_,
         other.header_size_ + other.header_->payload_size);
  write_offset_ = other.write_offset_;
  return *this;
}

void Pickle::WriteString(const StringPiece& value) {
  WriteLength(value.size());
  WriteBytes(value.data(), static_cast<int>(value.size()));
}

void Pickle::WriteString16(const StringPiece16& value) {
  WriteLength(value.size());
  WriteBytes(value.data(), static_cast<int>(value.size()) * sizeof(char16));
}

void Pickle::WriteData(const char* data, size_t length) {
  WriteLength(length); 
  WriteBytes(data, length);
}

void Pickle::WriteBytes(const void* data, size_t length) {
  WriteBytesCommon(data, length);
}

void Pickle::Reserve(size_t length) {
  size_t data_len = bits::AlignUp(length, sizeof(uint32_t));
  CR_DCHECK(data_len >= length);
#if defined(MINI_CHROMIUM_ARCH_CPU_64_BITS)
  CR_DCHECK(data_len <= std::numeric_limits<uint32_t>::max());
#endif
  CR_DCHECK(write_offset_ <= (std::numeric_limits<uint32_t>::max() - data_len));
  size_t new_size = write_offset_ + data_len;
  if (new_size > capacity_after_header_)
    Resize(capacity_after_header_ * 2 + new_size);
}

void Pickle::Resize(size_t new_capacity) {
  capacity_after_header_ = bits::AlignUp(new_capacity, kPayloadUnit);
  void* p = realloc(header_, GetTotalAllocatedSize());
  header_ = reinterpret_cast<Header*>(p);
}

void* Pickle::ClaimBytes(size_t num_bytes) {
  void* p = ClaimUninitializedBytesInternal(num_bytes);
  memset(p, 0, num_bytes);
  return p;
}

size_t Pickle::GetTotalAllocatedSize() const {
  if (capacity_after_header_ == kCapacityReadOnly)
    return 0;
  return header_size_ + capacity_after_header_;
}

// static
const char* Pickle::FindNext(size_t header_size,
                             const char* start,
                             const char* end) {
  size_t pickle_size = 0;
  if (!PeekNext(header_size, start, end, &pickle_size))
    return NULL;

  if (pickle_size > static_cast<size_t>(end - start))
    return NULL;

  return start + pickle_size;
}

// static
bool Pickle::PeekNext(size_t header_size,
                      const char* start,
                      const char* end,
                      size_t* pickle_size) {
  CR_DCHECK(header_size == bits::AlignUp(header_size, sizeof(uint32_t)));
  CR_DCHECK(header_size >= sizeof(Header));
  CR_DCHECK(header_size <= static_cast<size_t>(kPayloadUnit));

  size_t length = static_cast<size_t>(end - start);
  if (length < sizeof(Header))
    return false;

  const Header* hdr = reinterpret_cast<const Header*>(start);
  if (length < header_size)
    return false;

  if (hdr->payload_size > std::numeric_limits<size_t>::max() - header_size) {
    // If payload_size causes an overflow, we return maximum possible
    // pickle size to indicate that.
    *pickle_size = std::numeric_limits<size_t>::max();
  } else {
    *pickle_size = header_size + hdr->payload_size;
  }
  return true;
}

template <size_t length> void Pickle::WriteBytesStatic(const void* data) {
  WriteBytesCommon(data, length);
}

template void Pickle::WriteBytesStatic<1>(const void* data);
template void Pickle::WriteBytesStatic<2>(const void* data);
template void Pickle::WriteBytesStatic<4>(const void* data);
template void Pickle::WriteBytesStatic<8>(const void* data);

inline void* Pickle::ClaimUninitializedBytesInternal(size_t length) {
  CR_DCHECK(kCapacityReadOnly != capacity_after_header_)
      << "oops: pickle is readonly";
  size_t data_len = bits::AlignUp(length, sizeof(uint32_t));
  CR_DCHECK(data_len >= length);
#if defined(MINI_CHROMIUM_ARCH_CPU_64_BITS)
  CR_DCHECK(data_len <= std::numeric_limits<uint32_t>::max());
#endif
  CR_DCHECK(write_offset_ <= (std::numeric_limits<uint32_t>::max() - data_len));
  size_t new_size = write_offset_ + data_len;
  if (new_size > capacity_after_header_) {
    size_t new_capacity = capacity_after_header_ * 2;
    const size_t kPickleHeapAlign = 4096;
    if (new_capacity > kPickleHeapAlign)
      new_capacity = 
          bits::AlignUp(new_capacity, kPickleHeapAlign) - kPayloadUnit;
    Resize(std::max(new_capacity, new_size));
  }

  char* write = mutable_payload() + write_offset_;
  memset(write + length, 0, data_len - length);  // Always initialize padding
  header_->payload_size = static_cast<uint32_t>(new_size);
  write_offset_ = new_size;
  return write;
}

inline void Pickle::WriteBytesCommon(const void* data, size_t length) {
  CR_DCHECK(kCapacityReadOnly != capacity_after_header_)
      << "oops: pickle is readonly";
  void* write = ClaimUninitializedBytesInternal(length);
  memcpy(write, data, length);
}

}  // namespace cr

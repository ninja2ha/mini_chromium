/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "crbase/buffer/byte_buffer.h"

#include <string.h>

namespace cr {

ByteBufferWriter::ByteBufferWriter() : ByteBufferWriterT() {}

ByteBufferWriter::ByteBufferWriter(const char* bytes, size_t len)
    : ByteBufferWriterT(bytes, len) {}

ByteBufferReader::ByteBufferReader(const char* bytes, size_t len) {
  Construct(bytes, len);
}

ByteBufferReader::ByteBufferReader(const char* bytes) {
  Construct(bytes, strlen(bytes));
}

ByteBufferReader::ByteBufferReader(const ByteBuffer& buf) {
  Construct(buf.data<char>(), buf.size());
}

ByteBufferReader::ByteBufferReader(const ByteBufferWriter& buf) {
  Construct(buf.data(), buf.length());
}

void ByteBufferReader::Construct(const char* bytes, size_t len) {
  bytes_ = bytes;
  size_ = len;
  start_ = 0;
  end_ = len;
}

bool ByteBufferReader::ReadUInt8(uint8_t* val) {
  if (!val)
    return false;

  return ReadBytes(reinterpret_cast<char*>(val), 1);
}

bool ByteBufferReader::ReadUIntBE16(uint16_t* val) {
  if (!val)
    return false;

  uint16_t v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
    return false;
  } else {
    *val = cr::ByteSwapToBE16(v);
    return true;
  }
}

bool ByteBufferReader::ReadUIntBE24(uint32_t* val) {
  if (!val)
    return false;

  uint32_t v = 0;
  char* read_into = reinterpret_cast<char*>(&v);
  ++read_into;

  if (!ReadBytes(read_into, 3)) {
    return false;
  } else {
    *val = cr::ByteSwapToBE32(v);
    return true;
  }
}

bool ByteBufferReader::ReadUIntBE32(uint32_t* val) {
  if (!val)
    return false;

  uint32_t v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 4)) {
    return false;
  } else {
    *val = cr::ByteSwapToBE32(v);
    return true;
  }
}

bool ByteBufferReader::ReadUIntBE64(uint64_t* val) {
  if (!val)
    return false;

  uint64_t v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 8)) {
    return false;
  } else {
    *val = cr::ByteSwapToBE64(v);
    return true;
  }
}

bool ByteBufferReader::ReadUVarint(uint64_t* val) {
  if (!val) {
    return false;
  }
  // Integers are deserialized 7 bits at a time, with each byte having a
  // continuation byte (msb=1) if there are more bytes to be read.
  uint64_t v = 0;
  for (int i = 0; i < 64; i += 7) {
    char byte;
    if (!ReadBytes(&byte, 1)) {
      return false;
    }
    // Read the first 7 bits of the byte, then offset by bits read so far.
    v |= (static_cast<uint64_t>(byte) & 0x7F) << i;
    // True if the msb is not a continuation byte.
    if (static_cast<uint64_t>(byte) < 0x80) {
      *val = v;
      return true;
    }
  }
  return false;
}

bool ByteBufferReader::ReadUIntLE16(uint16_t* val) {
  if (!val)
    return false;

  uint16_t v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
    return false;
  } else {
    *val = cr::ByteSwapToLE16(v);
    return true;
  }
}

bool ByteBufferReader::ReadUIntLE24(uint32_t* val) {
  if (!val)
    return false;

  uint32_t v = 0;
  char* read_into = reinterpret_cast<char*>(&v);
  ++read_into;

  if (!ReadBytes(read_into, 3)) {
    return false;
  } else {
    *val = cr::ByteSwapToLE32(v);
    return true;
  }
}

bool ByteBufferReader::ReadUIntLE32(uint32_t* val) {
  if (!val)
    return false;

  uint32_t v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 4)) {
    return false;
  }
  else {
    *val = cr::ByteSwapToLE32(v);
    return true;
  }
}

bool ByteBufferReader::ReadUIntLE64(uint64_t* val) {
  if (!val)
    return false;

  uint64_t v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 8)) {
    return false;
  }
  else {
    *val = cr::ByteSwapToLE64(v);
    return true;
  }
}

bool ByteBufferReader::ReadBytes(char* val, size_t len) {
  if (len > length()) {
    return false;
  } else {
    memcpy(val, bytes_ + start_, len);
    start_ += len;
    return true;
  }
}

bool ByteBufferReader::ReadString(cr::StringPiece* val, size_t len) {
  if (!val)
    return false;

  if (len > length()) {
    return false;
  }
  else {
    val->set(bytes_ + start_, len);
    start_ += len;
    return true;
  }
}

bool ByteBufferReader::Consume(size_t size) {
  if (size > length())
    return false;
  start_ += size;
  return true;
}

}  // namespace cr

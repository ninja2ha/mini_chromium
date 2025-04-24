// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// NetToHostXX() / HostToNextXX() functions equivalent to the traditional 
// ntohX() and htonX() functions.
// Use the functions defined here rather than using the platform-specific
// functions directly.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_SYS_BYTEORDER_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_SYS_BYTEORDER_H_

#include <stdint.h>

#include "crbase/byte_order.h"

namespace crnet {

// Converts the bytes in |x| from network to host order (endianness), and
// returns the result.
inline uint16_t NetToHost16(uint16_t x) {
  return cr::ByteSwapToBE16(x);
}

inline uint32_t NetToHost32(uint32_t x) {
  return cr::ByteSwapToBE32(x);
}

inline uint64_t NetToHost64(uint64_t x) {
  return cr::ByteSwapToBE64(x);
}

// Converts the bytes in |x| from host to network order (endianness), and
// returns the result.
inline uint16_t HostToNet16(uint16_t x) {
  return cr::ByteSwapToBE16(x);
}

inline uint32_t HostToNet32(uint32_t x) {
  return cr::ByteSwapToBE32(x);
}

inline uint64_t HostToNet64(uint64_t x) {
  return cr::ByteSwapToBE64(x);
}

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_SYS_BYTEORDER_H_
// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/files/memory_mapped_file.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "crbase/logging/logging.h"
#include "crbase/files/file_util.h"
#include "crbase/numerics/safe_conversions.h"
///#include "crbase/threading/scoped_blocking_call.h"
#include "crbuild/build_config.h"

namespace cr {

MemoryMappedFile::MemoryMappedFile() : data_(nullptr), length_(0) {}

bool MemoryMappedFile::MapFileRegionToMemory(
    const MemoryMappedFile::Region& region,
    Access access) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  off_t map_start = 0;
  size_t map_size = 0;
  int32_t data_offset = 0;

  if (region == MemoryMappedFile::Region::kWholeFile) {
    int64_t file_len = file_.GetLength();
    if (file_len < 0) {
      CR_DPLOG(Error) << "fstat " << file_.GetPlatformFile();
      return false;
    }
    if (!IsValueInRangeForNumericType<size_t>(file_len))
      return false;
    map_size = static_cast<size_t>(file_len);
    length_ = map_size;
  } else {
    // The region can be arbitrarily aligned. mmap, instead, requires both the
    // start and size to be page-aligned. Hence, we map here the page-aligned
    // outer region [|aligned_start|, |aligned_start| + |size|] which contains
    // |region| and then add up the |data_offset| displacement.
    int64_t aligned_start = 0;
    size_t aligned_size = 0;
    CalculateVMAlignedBoundaries(region.offset,
                                 region.size,
                                 &aligned_start,
                                 &aligned_size,
                                 &data_offset);

    // Ensure that the casts in the mmap call below are sane.
    if (aligned_start < 0 ||
        !IsValueInRangeForNumericType<off_t>(aligned_start)) {
      CR_DLOG(Error) << "Region bounds are not valid for mmap";
      return false;
    }

    map_start = static_cast<off_t>(aligned_start);
    map_size = aligned_size;
    length_ = region.size;
  }

  int flags = 0;
  switch (access) {
    case READ_ONLY:
      flags |= PROT_READ;
      break;

    case READ_WRITE:
      flags |= PROT_READ | PROT_WRITE;
      break;

    case READ_WRITE_EXTEND:
      flags |= PROT_READ | PROT_WRITE;

      if (!AllocateFileRegion(&file_, region.offset, region.size))
        return false;

      break;
  }

  data_ = static_cast<uint8_t*>(mmap(nullptr, map_size, flags, MAP_SHARED,
                                     file_.GetPlatformFile(), map_start));
  if (data_ == MAP_FAILED) {
    CR_DPLOG(Error) << "mmap " << file_.GetPlatformFile();
    return false;
  }

  data_ += data_offset;
  return true;
}

void MemoryMappedFile::CloseHandles() {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  if (data_ != nullptr)
    munmap(data_, length_);
  file_.Close();

  data_ = nullptr;
  length_ = 0;
}

}  // namespace cr
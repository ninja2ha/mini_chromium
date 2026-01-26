// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/files/memory_mapped_file.h"

#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/files/file_path.h"
#include "crbase/numerics/safe_math.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/windows_version.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#endif

namespace cr {

const MemoryMappedFile::Region MemoryMappedFile::Region::kWholeFile = {0, 0};

bool MemoryMappedFile::Region::operator==(
    const MemoryMappedFile::Region& other) const {
  return other.offset == offset && other.size == size;
}

bool MemoryMappedFile::Region::operator!=(
    const MemoryMappedFile::Region& other) const {
  return other.offset != offset || other.size != size;
}

MemoryMappedFile::~MemoryMappedFile() {
  CloseHandles();
}

bool MemoryMappedFile::Initialize(const FilePath& file_name, Access access) {
  if (IsValid())
    return false;

  uint32_t flags = 0;
  switch (access) {
    case READ_ONLY:
      flags = File::FLAG_OPEN | File::FLAG_READ;
      break;
    case READ_WRITE:
      flags = File::FLAG_OPEN | File::FLAG_READ | File::FLAG_WRITE;
      break;
    case READ_WRITE_EXTEND:
      // Can't open with "extend" because no maximum size is known.
      CR_NOTREACHED();
      break;
#if defined(MINI_CHROMIUM_OS_WIN)
    case READ_CODE_IMAGE:
      flags |= File::FLAG_OPEN | File::FLAG_READ | File::FLAG_EXCLUSIVE_WRITE |
               File::FLAG_EXECUTE;
      break;
#endif
  }
  file_.Initialize(file_name, flags);

  if (!file_.IsValid()) {
    CR_DLOG(Error) << "Couldn't open " << file_name.AsUTF8Unsafe();
    return false;
  }

  if (!MapFileRegionToMemory(Region::kWholeFile, access)) {
    CloseHandles();
    return false;
  }

  return true;
}

bool MemoryMappedFile::Initialize(File file, Access access) {
  CR_DCHECK(READ_WRITE_EXTEND != access);
  return Initialize(std::move(file), Region::kWholeFile, access);
}

bool MemoryMappedFile::Initialize(File file,
                                  const Region& region,
                                  Access access) {
  switch (access) {
    case READ_WRITE_EXTEND:
      CR_DCHECK(Region::kWholeFile != region);
      {
        CheckedNumeric<int64_t> region_end(region.offset);
        region_end += region.size;
        if (!region_end.IsValid()) {
          CR_DLOG(Error) << "Region bounds exceed maximum for cr::File.";
          return false;
        }
      }
      CR_FALLTHROUGH;
    case READ_ONLY:
    case READ_WRITE:
      // Ensure that the region values are valid.
      if (region.offset < 0) {
        CR_DLOG(Error) << "Region bounds are not valid.";
        return false;
      }
      break;
#if defined(MINI_CHROMIUM_OS_WIN)
    case READ_CODE_IMAGE:
      // Can't open with "READ_CODE_IMAGE", not supported outside Windows
      // or with a |region|.
      CR_NOTREACHED();
      break;
#endif
  }

  if (IsValid())
    return false;

  if (region != Region::kWholeFile)
    CR_DCHECK(region.offset >= 0);

  file_ = std::move(file);

  if (!MapFileRegionToMemory(region, access)) {
    CloseHandles();
    return false;
  }

  return true;
}

bool MemoryMappedFile::IsValid() const {
  return data_ != nullptr;
}

// static
void MemoryMappedFile::CalculateVMAlignedBoundaries(int64_t start,
                                                    size_t size,
                                                    int64_t* aligned_start,
                                                    size_t* aligned_size,
                                                    int32_t* offset) {
  // Sadly, on Windows, the mmap alignment is not just equal to the page size.
#if defined(MINI_CHROMIUM_OS_WIN)
  auto mask = win::OSInfo::GetInstance()->allocation_granularity();
#elif defined(MINI_CHROMIUM_OS_POSIX)
  auto mask = getpagesize();
#else
#error Unsupport platform.
#endif
  mask--;
  CR_CHECK(IsValueInRangeForNumericType<int32_t>(mask));

  *offset = static_cast<int32_t>(static_cast<uint64_t>(start) & mask);
  *aligned_start = static_cast<int64_t>(static_cast<uint64_t>(start) & ~mask);
  // The DCHECK above means bit 31 is not set in `mask`, which in turn means
  // *offset is positive.  Therefore casting it to a size_t is safe.
  *aligned_size =
      (size + static_cast<size_t>(*offset) + static_cast<size_t>(mask)) & ~mask;
}

}  // namespace cr
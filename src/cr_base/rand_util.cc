// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/rand_util.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>

#include <algorithm>
#include <limits>

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif 
typedef struct IUnknown IUnknown;
#include <windows.h>
// #define needed to link in RtlGenRandom(), a.k.a. SystemFunction036.  See the
// "Community Additions" comment on MSDN here:
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa387694.aspx
#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#endif  // defined(MINI_CHROMIUM_OS_WIN)


#include "cr_base/logging/logging.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/memory/no_destructor.h"

namespace cr {

#if defined(MINI_CHROMIUM_OS_POSIX)
namespace {

static constexpr int kOpenFlags = O_RDONLY | O_CLOEXEC;

// We keep the file descriptor for /dev/urandom around so we don't need to
// reopen it (which is expensive), and since we may not even be able to reopen
// it if we are later put in a sandbox. This class wraps the file descriptor so
// we can use a static-local variable to handle opening it on the first access.
class URandomFd {
 public:
  URandomFd() : fd_(HANDLE_EINTR(open("/dev/urandom", kOpenFlags))) {
    CR_CHECK(fd_ >= 0) << "Cannot open /dev/urandom";
  }

  ~URandomFd() { close(fd_); }

  int fd() const { return fd_; }

 private:
  const int fd_;
};

// Copied from file_util_posix.cc.
bool ReadFromFD(int fd, char* buffer, size_t bytes) {
  size_t total_read = 0;
  while (total_read < bytes) {
    ssize_t bytes_read =
        HANDLE_EINTR(read(fd, buffer + total_read, bytes - total_read));
    if (bytes_read <= 0)
      break;
    total_read += bytes_read;
  }
  return total_read == bytes;
}

int GetUrandomFD() {
  static NoDestructor<URandomFd> urandom_fd;
  return urandom_fd->fd();
}

}  // namespace
#endif

void RandBytes(void* output, size_t output_length) {
#if defined(MINI_CHROMIUM_OS_WIN)
  char* output_ptr = static_cast<char*>(output);
  while (output_length > 0) {
    const ULONG output_bytes_this_pass = static_cast<ULONG>(std::min(
        output_length, static_cast<size_t>(std::numeric_limits<ULONG>::max())));
    const bool success =
        RtlGenRandom(output_ptr, output_bytes_this_pass) != FALSE;
    CR_CHECK(success);
    output_length -= output_bytes_this_pass;
    output_ptr += output_bytes_this_pass;
  }

#elif defined(MINI_CHROMIUM_OS_POSIX)
  // If the OS-specific mechanisms didn't work, fall through to reading from
  // urandom.
  //
  // TODO(crbug.com/995996): When we no longer need to support old Linux
  // kernels, we can get rid of this /dev/urandom branch altogether.
  const int urandom_fd = GetUrandomFD();
  const bool success =
      ReadFromFD(urandom_fd, static_cast<char*>(output), output_length);
  CR_CHECK(success);
#endif
}

uint64_t RandUint64() {
  uint64_t number;
  RandBytes(&number, sizeof(number));
  return number;
}

int RandInt(int min, int max) {
  CR_DCHECK(min <= max);

  uint64_t range = static_cast<uint64_t>(max) - min + 1;
  // |range| is at most UINT_MAX + 1, so the result of RandGenerator(range)
  // is at most UINT_MAX.  Hence it's safe to cast it from uint64_t to int64_t.
  int result =
      static_cast<int>(min + static_cast<int64_t>(cr::RandGenerator(range)));
  CR_DCHECK(result >= min);
  CR_DCHECK(result <= max);
  return result;
}

double RandDouble() {
  return BitsToOpenEndedUnitInterval(cr::RandUint64());
}

double BitsToOpenEndedUnitInterval(uint64_t bits) {
  // We try to get maximum precision by masking out as many bits as will fit
  // in the target type's mantissa, and raising it to an appropriate power to
  // produce output in the range [0, 1).  For IEEE 754 doubles, the mantissa
  // is expected to accommodate 53 bits.

  static_assert(std::numeric_limits<double>::radix == 2,
                "otherwise use scalbn");
  static const int kBits = std::numeric_limits<double>::digits;
  uint64_t random_bits = bits & ((UINT64_C(1) << kBits) - 1);
  double result = ldexp(static_cast<double>(random_bits), -1 * kBits);
  CR_DCHECK(result >= 0.0);
  CR_DCHECK(result < 1.0);
  return result;
}

uint64_t RandGenerator(uint64_t range) {
  CR_DCHECK(range > 0u);
  // We must discard random results above this number, as they would
  // make the random generator non-uniform (consider e.g. if
  // MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
  // as likely as a result of 3 or 4).
  uint64_t max_acceptable_value =
      (std::numeric_limits<uint64_t>::max() / range) * range - 1;

  uint64_t value;
  do {
    value = cr::RandUint64();
  } while (value > max_acceptable_value);

  return value % range;
}

std::string RandBytesAsString(size_t length) {
  CR_DCHECK(length > 0u);
  std::string result;
  RandBytes(WriteInto(&result, length + 1), length);
  return result;
}

}  // namespace cr
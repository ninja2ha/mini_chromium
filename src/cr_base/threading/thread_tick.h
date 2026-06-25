// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/base_export.h"

#include "cr_base/time/time.h"

namespace cr {

// Represents a clock, specific to a particular thread, than runs only while the
// thread is running.
class CRBASE_EXPORT ThreadTicks : public time_internal::TimeBase<ThreadTicks> {
 public:
  constexpr ThreadTicks() : TimeBase(0) {}

  // Returns true if ThreadTicks::Now() is supported on this system.
  static bool IsSupported() CR_WARN_UNUSED_RESULT {
#if (defined(_POSIX_THREAD_CPUTIME) && (_POSIX_THREAD_CPUTIME >= 0))
    return true;
#elif defined(MINI_CHROMIUM_OS_WIN)
    return IsSupportedWin();
#else
    return false;
#endif
  }

  // Waits until the initialization is completed. Needs to be guarded with a
  // call to IsSupported().
  static void WaitUntilInitialized() {
#if defined(MINI_CHROMIUM_OS_WIN)
    WaitUntilInitializedWin();
#endif
  }

  // Returns thread-specific CPU-time on systems that support this feature.
  // Needs to be guarded with a call to IsSupported(). Use this timer
  // to (approximately) measure how much time the calling thread spent doing
  // actual work vs. being de-scheduled. May return bogus results if the thread
  // migrates to another CPU between two calls. Returns an empty ThreadTicks
  // object until the initialization is completed. If a clock reading is
  // absolutely needed, call WaitUntilInitialized() before this method.
  static ThreadTicks Now();

#if defined(MINI_CHROMIUM_OS_WIN)
  // Similar to Now() above except this returns thread-specific CPU time for an
  // arbitrary thread. All comments for Now() method above apply apply to this
  // method as well.
  static ThreadTicks GetForThread(HANDLE thread_handle);
#endif

  // Converts an integer value representing ThreadTicks to a class. This may be
  // used when deserializing a |ThreadTicks| structure, using a value known to
  // be compatible. It is not provided as a constructor because the integer type
  // may be unclear from the perspective of a caller.
  //
  // DEPRECATED - Do not use in new code. For deserializing ThreadTicks values,
  // prefer ThreadTicks + TimeDelta(); however, be aware that the origin is not
  // fixed and may vary. Serializing for persistence is strongly
  // discouraged. http://crbug.com/634507
  static constexpr ThreadTicks FromInternalValue(int64_t us) {
    return ThreadTicks(us);
  }

 private:
  friend class time_internal::TimeBase<ThreadTicks>;

  // Please use Now() or GetForThread() to create a new object. This is for
  // internal use and testing.
  constexpr explicit ThreadTicks(int64_t us) : TimeBase(us) {}

#if defined(MINI_CHROMIUM_OS_WIN)
  ///CR_FRIEND_GTEST_ALL_PREFIXES(TimeTicks, TSCTicksPerSecond);

#if defined(MINI_CHROMIUM_ARCH_CPU_ARM64)
  // TSCTicksPerSecond is not supported on Windows on Arm systems because the
  // cycle-counting methods use the actual CPU cycle count, and not a consistent
  // incrementing counter.
#else
  // Returns the frequency of the TSC in ticks per second, or 0 if it hasn't
  // been measured yet. Needs to be guarded with a call to IsSupported().
  // This method is declared here rather than in the anonymous namespace to
  // allow testing.
  static double TSCTicksPerSecond();
#endif

  static bool IsSupportedWin() CR_WARN_UNUSED_RESULT;
  static void WaitUntilInitializedWin();
#endif
};

// For logging use only.
CRBASE_EXPORT std::ostream& operator<<(std::ostream& os, ThreadTicks time_ticks);

}  // namespace cr

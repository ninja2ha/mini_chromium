// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/threading/thread_tick.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
typedef struct IUnknown IUnknown;
#include <windows.h>

#include "cr_base/system/cpu_info.h"

namespace cr {

namespace {

// Returns the current value of the performance counter.
uint64_t QPCNowRaw() {
  LARGE_INTEGER perf_counter_now = {};
  // According to the MSDN documentation for QueryPerformanceCounter(), this
  // will never fail on systems that run XP or later.
  // https://msdn.microsoft.com/library/windows/desktop/ms644904.aspx
  ::QueryPerformanceCounter(&perf_counter_now);
  return perf_counter_now.QuadPart;
}

}  // namespace

// static
ThreadTicks ThreadTicks::Now() {
  return ThreadTicks::GetForThread(::GetCurrentThread());
}

// static
ThreadTicks ThreadTicks::GetForThread(HANDLE thread_handle) {
  CR_DCHECK(IsSupported());

#if defined(MINI_CHROMIUM_ARCH_CPU_ARM64)
  // QueryThreadCycleTime versus TSCTicksPerSecond doesn't have much relation to
  // actual elapsed time on Windows on Arm, because QueryThreadCycleTime is
  // backed by the actual number of CPU cycles executed, rather than a
  // constant-rate timer like Intel. To work around this, use GetThreadTimes
  // (which isn't as accurate but is meaningful as a measure of elapsed
  // per-thread time).
  FILETIME creation_time, exit_time, kernel_time, user_time;
  ::GetThreadTimes(thread_handle, &creation_time, &exit_time,
                   &kernel_time, &user_time);

  const int64_t us = FileTimeToMicroseconds(user_time);
#else
  // Get the number of TSC ticks used by the current thread.
  ULONG64 thread_cycle_time = 0;
  ::QueryThreadCycleTime(thread_handle, &thread_cycle_time);

  // Get the frequency of the TSC.
  const double tsc_ticks_per_second = TSCTicksPerSecond();
  if (tsc_ticks_per_second == 0)
    return ThreadTicks();

  // Return the CPU time of the current thread.
  const double thread_time_seconds = thread_cycle_time / tsc_ticks_per_second;
  const int64_t us =
      static_cast<int64_t>(thread_time_seconds * Time::kMicrosecondsPerSecond);
#endif

  return ThreadTicks(us);
}

// static
bool ThreadTicks::IsSupportedWin() {
  static bool is_supported = CPUInfo().has_non_stop_time_stamp_counter();
  return is_supported;
}

// static
void ThreadTicks::WaitUntilInitializedWin() {
#if !defined(MINI_CHROMIUM_ARCH_CPU_ARM64)
  while (TSCTicksPerSecond() == 0)
    ::Sleep(10);
#endif
}

#if !defined(MINI_CHROMIUM_ARCH_CPU_ARM64)
double ThreadTicks::TSCTicksPerSecond() {
  CR_DCHECK(IsSupported());
  // The value returned by QueryPerformanceFrequency() cannot be used as the TSC
  // frequency, because there is no guarantee that the TSC frequency is equal to
  // the performance counter frequency.
  // The TSC frequency is cached in a static variable because it takes some time
  // to compute it.
  static double tsc_ticks_per_second = 0;
  if (tsc_ticks_per_second != 0)
    return tsc_ticks_per_second;

  // Increase the thread priority to reduces the chances of having a context
  // switch during a reading of the TSC and the performance counter.
  const int previous_priority = ::GetThreadPriority(::GetCurrentThread());
  ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

  // The first time that this function is called, make an initial reading of the
  // TSC and the performance counter.

  static const uint64_t tsc_initial = __rdtsc();
  static const uint64_t perf_counter_initial = QPCNowRaw();

  // Make a another reading of the TSC and the performance counter every time
  // that this function is called.
  const uint64_t tsc_now = __rdtsc();
  const uint64_t perf_counter_now = QPCNowRaw();

  // Reset the thread priority.
  ::SetThreadPriority(::GetCurrentThread(), previous_priority);

  // Make sure that at least 50 ms elapsed between the 2 readings. The first
  // time that this function is called, we don't expect this to be the case.
  // Note: The longer the elapsed time between the 2 readings is, the more
  //   accurate the computed TSC frequency will be. The 50 ms value was
  //   chosen because local benchmarks show that it allows us to get a
  //   stddev of less than 1 tick/us between multiple runs.
  // Note: According to the MSDN documentation for QueryPerformanceFrequency(),
  //   this will never fail on systems that run XP or later.
  //   https://msdn.microsoft.com/library/windows/desktop/ms644905.aspx
  LARGE_INTEGER perf_counter_frequency = {};
  ::QueryPerformanceFrequency(&perf_counter_frequency);
  CR_DCHECK(perf_counter_now >= perf_counter_initial);
  const uint64_t perf_counter_ticks = perf_counter_now - perf_counter_initial;
  const double elapsed_time_seconds =
      perf_counter_ticks / static_cast<double>(perf_counter_frequency.QuadPart);

  constexpr double kMinimumEvaluationPeriodSeconds = 0.05;
  if (elapsed_time_seconds < kMinimumEvaluationPeriodSeconds)
    return 0;

  // Compute the frequency of the TSC.
  CR_DCHECK(tsc_now >= tsc_initial);
  const uint64_t tsc_ticks = tsc_now - tsc_initial;
  tsc_ticks_per_second = tsc_ticks / elapsed_time_seconds;

  return tsc_ticks_per_second;
}
#endif  // defined(ARCH_CPU_ARM64)

}  // namespace 
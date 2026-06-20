// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

// Windows Timer Primer
//
// A good article:  http://www.ddj.com/windows/184416651
// A good mozilla bug:  http://bugzilla.mozilla.org/show_bug.cgi?id=363258
//
// The default windows timer, GetSystemTimeAsFileTime is not very precise.
// It is only good to ~15.5ms.
//
// QueryPerformanceCounter is the logical choice for a high-precision timer.
// However, it is known to be buggy on some hardware.  Specifically, it can
// sometimes "jump".  On laptops, QPC can also be very expensive to call.
// It's 3-4x slower than timeGetTime() on desktops, but can be 10x slower
// on laptops.  A unittest exists which will show the relative cost of various
// timers on any system.
//
// The next logical choice is timeGetTime().  timeGetTime has a precision of
// 1ms, but only if you call APIs (timeBeginPeriod()) which affect all other
// applications on the system.  By default, precision is only 15.5ms.
// Unfortunately, we don't want to call timeBeginPeriod because we don't
// want to affect other applications.  Further, on mobile platforms, use of
// faster multimedia timers can hurt battery life.  See the intel
// article about this here:
// http://softwarecommunity.intel.com/articles/eng/1086.htm
//
// To work around all this, we're going to generally use timeGetTime().  We
// will only increase the system-wide timer if we're not running on battery
// power.

#include "cr_base/time/time.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif  //

// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>
#include <mmsystem.h>

#include <stdint.h>

#include <atomic>

#include "cr_base/logging/logging.h"
#include "cr_base/atomic/atomicops.h"
#include "cr_base/bit_cast.h"
#include "cr_base/system/cpu_info.h"
#include "cr_base/synchronization/lock.h"

namespace cr {

namespace {

// From MSDN, FILETIME "Contains a 64-bit value representing the number of
// 100-nanosecond intervals since January 1, 1601 (UTC)."
int64_t FileTimeToMicroseconds(const FILETIME& ft) {
  // Need to bit_cast to fix alignment, then divide by 10 to convert
  // 100-nanoseconds to microseconds. This only works on little-endian
  // machines.
  return cr::BitCast<int64_t, FILETIME>(ft) / 10;
}

bool CanConvertToFileTime(int64_t us) {
  return us >= 0 && us <= (std::numeric_limits<int64_t>::max() / 10);
}

void MicrosecondsToFileTime(int64_t us, FILETIME* ft) {
  CR_DCHECK(CanConvertToFileTime(us)) 
      << "Out-of-range: Cannot convert " << us
      << " microseconds to FILETIME units.";

  // Multiply by 10 to convert microseconds to 100-nanoseconds. Bit_cast will
  // handle alignment problems. This only works on little-endian machines.
  *ft = cr::BitCast<FILETIME, int64_t>(us * 10);
}

int64_t CurrentWallclockMicroseconds() {
  FILETIME ft;
  ::GetSystemTimeAsFileTime(&ft);
  return FileTimeToMicroseconds(ft);
}

// Time between resampling the un-granular clock for this API.
constexpr TimeDelta kMaxTimeToAvoidDrift = TimeDelta::FromSeconds(60);

int64_t g_initial_time = 0;
TimeTicks g_initial_ticks;

void InitializeClock() {
  g_initial_ticks = TimeTicks::Now();
  g_initial_time = CurrentWallclockMicroseconds();
}

// Track the last value passed to timeBeginPeriod so that we can cancel that
// call by calling timeEndPeriod with the same value. A value of zero means that
// the timer frequency is not currently raised.
UINT g_last_interval_requested_ms = 0;
// Track if kMinTimerIntervalHighResMs or kMinTimerIntervalLowResMs is active.
// For most purposes this could also be named g_is_on_ac_power.
bool g_high_res_timer_enabled = false;
// How many times the high resolution timer has been called.
uint32_t g_high_res_timer_count = 0;
// Start time of the high resolution timer usage monitoring. This is needed
// to calculate the usage as percentage of the total elapsed time.
TimeTicks g_high_res_timer_usage_start;
// The cumulative time the high resolution timer has been in use since
// |g_high_res_timer_usage_start| moment.
TimeDelta g_high_res_timer_usage;
// Timestamp of the last activation change of the high resolution timer. This
// is used to calculate the cumulative usage.
TimeTicks g_high_res_timer_last_activation;
// The lock to control access to the above set of variables.
Lock* GetHighResLock() {
  static auto* lock = new Lock();
  return lock;
}

// The two values that ActivateHighResolutionTimer uses to set the systemwide
// timer interrupt frequency on Windows. These control how precise timers are
// but also have a big impact on battery life.

// Used when a faster timer has been requested (g_high_res_timer_count > 0) and
// the computer is running on AC power (plugged in) so that it's okay to go to
// the highest frequency.
constexpr UINT kMinTimerIntervalHighResMs = 1;

// Used when a faster timer has been requested (g_high_res_timer_count > 0) and
// the computer is running on DC power (battery) so that we don't want to raise
// the timer frequency as much.
constexpr UINT kMinTimerIntervalLowResMs = 8;

// Calculate the desired timer interrupt interval. Note that zero means that the
// system default should be used.
UINT GetIntervalMs() {
  if (!g_high_res_timer_count)
    return 0;  // Use the default, typically 15.625
  if (g_high_res_timer_enabled)
    return kMinTimerIntervalHighResMs;
  return kMinTimerIntervalLowResMs;
}

// Compare the currently requested timer interrupt interval to the last interval
// requested and update if necessary (by cancelling the old request and making a
// new request). If there is no change then do nothing.
void UpdateTimerIntervalLocked() {
  UINT new_interval = GetIntervalMs();
  if (new_interval == g_last_interval_requested_ms)
    return;
  if (g_last_interval_requested_ms) {
    // Record how long the timer interrupt frequency was raised.
    g_high_res_timer_usage += TimeTicks::Now() -
                              g_high_res_timer_last_activation;
    // Reset the timer interrupt back to the default.
    timeEndPeriod(g_last_interval_requested_ms);
  }
  g_last_interval_requested_ms = new_interval;
  if (g_last_interval_requested_ms) {
    // Record when the timer interrupt was raised.
    g_high_res_timer_last_activation = TimeTicks::Now();
    timeBeginPeriod(g_last_interval_requested_ms);
  }
}

// Returns the current value of the performance counter.
uint64_t QPCNowRaw() {
  LARGE_INTEGER perf_counter_now = {};
  // According to the MSDN documentation for QueryPerformanceCounter(), this
  // will never fail on systems that run XP or later.
  // https://msdn.microsoft.com/library/windows/desktop/ms644904.aspx
  ::QueryPerformanceCounter(&perf_counter_now);
  return perf_counter_now.QuadPart;
}

bool SafeConvertToWord(int in, WORD* out) {
  CheckedNumeric<WORD> result = in;
  *out = result.ValueOrDefault(std::numeric_limits<WORD>::max());
  return result.IsValid();
}

}  // namespace

// Time -----------------------------------------------------------------------

Time Time::Now() {
  if (g_initial_time == 0)
    InitializeClock();

  // We implement time using the high-resolution timers so that we can get
  // timeouts which are smaller than 10-15ms.  If we just used
  // CurrentWallclockMicroseconds(), we'd have the less-granular timer.
  //
  // To make this work, we initialize the clock (g_initial_time) and the
  // counter (initial_ctr).  To compute the initial time, we can check
  // the number of ticks that have elapsed, and compute the delta.
  //
  // To avoid any drift, we periodically resync the counters to the system
  // clock.
  while (true) {
    TimeTicks ticks = TimeTicks::Now();

    // Calculate the time elapsed since we started our timer
    TimeDelta elapsed = ticks - g_initial_ticks;

    // Check if enough time has elapsed that we need to resync the clock.
    if (elapsed > kMaxTimeToAvoidDrift) {
      InitializeClock();
      continue;
    }

    return Time() + elapsed + TimeDelta::FromMicroseconds(g_initial_time);
  }
}

Time Time::NowFromSystemTime() {
  // Force resync.
  InitializeClock();
  return Time() + TimeDelta::FromMicroseconds(g_initial_time);
}

// static
Time Time::FromFileTime(const FILETIME* ft) {
  if (cr::BitCast<int64_t, FILETIME>(*ft) == 0)
    return Time();

  if (ft->dwHighDateTime == std::numeric_limits<DWORD>::max() &&
      ft->dwLowDateTime == std::numeric_limits<DWORD>::max())
    return Max();

  return Time(FileTimeToMicroseconds(*ft));
}

void Time::ToFileTime(FILETIME* ft) const {
  if (is_null()) {
    *ft = BitCast<FILETIME, int64_t>(0);
    return;
  }

  if (is_max()) {
    ft->dwHighDateTime = std::numeric_limits<DWORD>::max();
    ft->dwLowDateTime = std::numeric_limits<DWORD>::max();

    return;
  }

  MicrosecondsToFileTime(us_, ft);
  return ;
}

// static
// Enable raising of the system-global timer interrupt frequency to 1 kHz (when
// enable is true, which happens when on AC power) or some lower frequency when
// on battery power (when enable is false). If the g_high_res_timer_enabled
// setting hasn't actually changed or if if there are no outstanding requests
// (if g_high_res_timer_count is zero) then do nothing.
// TL;DR - call this when going from AC to DC power or vice-versa.
void Time::EnableHighResolutionTimer(bool enable) {
  AutoLock lock(*GetHighResLock());
  g_high_res_timer_enabled = enable;
  UpdateTimerIntervalLocked();
}

// static
// Request that the system-global Windows timer interrupt frequency be raised.
// How high the frequency is raised depends on the system's power state and
// possibly other options.
// TL;DR - call this at the beginning and end of a time period where you want
// higher frequency timer interrupts. Each call with activating=true must be
// paired with a subsequent activating=false call.
bool Time::ActivateHighResolutionTimer(bool activating) {
  // We only do work on the transition from zero to one or one to zero so we
  // can easily undo the effect (if necessary) when EnableHighResolutionTimer is
  // called.
  const uint32_t max = std::numeric_limits<uint32_t>::max();

  AutoLock lock(*GetHighResLock());
  if (activating) {
    CR_DCHECK(g_high_res_timer_count != max);
    ++g_high_res_timer_count;
  } else {
    CR_DCHECK(g_high_res_timer_count != 0u);
    --g_high_res_timer_count;
  }
  UpdateTimerIntervalLocked();
  return true;
}

// static
// See if the timer interrupt interval has been set to the lowest value.
bool Time::IsHighResolutionTimerInUse() {
  AutoLock lock(*GetHighResLock());
  return g_last_interval_requested_ms == kMinTimerIntervalHighResMs;
}

// static
void Time::ResetHighResolutionTimerUsage() {
  AutoLock lock(*GetHighResLock());
  g_high_res_timer_usage = TimeDelta();
  g_high_res_timer_usage_start = TimeTicks::Now();
  if (g_high_res_timer_count > 0)
    g_high_res_timer_last_activation = g_high_res_timer_usage_start;
}

// static
double Time::GetHighResolutionTimerUsage() {
  AutoLock lock(*GetHighResLock());
  TimeTicks now = TimeTicks::Now();
  TimeDelta elapsed_time = now - g_high_res_timer_usage_start;
  if (elapsed_time.is_zero()) {
    // This is unexpected but possible if TimeTicks resolution is low and
    // GetHighResolutionTimerUsage() is called promptly after
    // ResetHighResolutionTimerUsage().
    return 0.0;
  }
  TimeDelta used_time = g_high_res_timer_usage;
  if (g_high_res_timer_count > 0) {
    // If currently activated add the remainder of time since the last
    // activation.
    used_time += now - g_high_res_timer_last_activation;
  }
  return used_time / elapsed_time * 100;
}

// static
bool Time::FromExploded(bool is_local, const Exploded& exploded, Time* time) {
  // Create the system struct representing our exploded time. It will either be
  // in local time or UTC.If casting from int to WORD results in overflow,
  // fail and return Time(0).
  SYSTEMTIME st;
  if (!SafeConvertToWord(exploded.year, &st.wYear) ||
      !SafeConvertToWord(exploded.month, &st.wMonth) ||
      !SafeConvertToWord(exploded.day_of_week, &st.wDayOfWeek) ||
      !SafeConvertToWord(exploded.day_of_month, &st.wDay) ||
      !SafeConvertToWord(exploded.hour, &st.wHour) ||
      !SafeConvertToWord(exploded.minute, &st.wMinute) ||
      !SafeConvertToWord(exploded.second, &st.wSecond) ||
      !SafeConvertToWord(exploded.millisecond, &st.wMilliseconds)) {
    *time = Time(0);
    return false;
  }

  FILETIME ft;
  bool success = true;
  // Ensure that it's in UTC.
  if (is_local) {
    SYSTEMTIME utc_st;
    success = TzSpecificLocalTimeToSystemTime(nullptr, &st, &utc_st) &&
              SystemTimeToFileTime(&utc_st, &ft);
  } else {
    success = !!SystemTimeToFileTime(&st, &ft);
  }

  *time = Time(success ? FileTimeToMicroseconds(ft) : 0);
  return success;
}

void Time::Explode(bool is_local, Exploded* exploded) const {
  if (!CanConvertToFileTime(us_)) {
    // We are not able to convert it to FILETIME.
    ZeroMemory(exploded, sizeof(*exploded));
    return;
  }

  FILETIME utc_ft;
  MicrosecondsToFileTime(us_, &utc_ft);

  // FILETIME in local time if necessary.
  bool success = true;
  // FILETIME in SYSTEMTIME (exploded).
  SYSTEMTIME st = {0};
  if (is_local) {
    SYSTEMTIME utc_st;
    // We don't use FileTimeToLocalFileTime here, since it uses the current
    // settings for the time zone and daylight saving time. Therefore, if it is
    // daylight saving time, it will take daylight saving time into account,
    // even if the time you are converting is in standard time.
    success = FileTimeToSystemTime(&utc_ft, &utc_st) &&
              SystemTimeToTzSpecificLocalTime(nullptr, &utc_st, &st);
  } else {
    success = !!FileTimeToSystemTime(&utc_ft, &st);
  }

  if (!success) {
    ZeroMemory(exploded, sizeof(*exploded));
    return;
  }

  exploded->year = st.wYear;
  exploded->month = st.wMonth;
  exploded->day_of_week = st.wDayOfWeek;
  exploded->day_of_month = st.wDay;
  exploded->hour = st.wHour;
  exploded->minute = st.wMinute;
  exploded->second = st.wSecond;
  exploded->millisecond = st.wMilliseconds;
}

// TimeTicks ------------------------------------------------------------------

namespace {

// We define a wrapper to adapt between the __stdcall and __cdecl call of the
// mock function, and to avoid a static constructor.  Assigning an import to a
// function pointer directly would require setup code to fetch from the IAT.
DWORD timeGetTimeWrapper() {
  return timeGetTime();
}

DWORD (*g_tick_function)(void) = &timeGetTimeWrapper;

// A structure holding the most significant bits of "last seen" and a
// "rollover" counter.
union LastTimeAndRolloversState {
  // The state as a single 32-bit opaque value.
  subtle::Atomic32 as_opaque_32;

  // The state as usable values.
  struct {
    // The top 8-bits of the "last" time. This is enough to check for rollovers
    // and the small bit-size means fewer CompareAndSwap operations to store
    // changes in state, which in turn makes for fewer retries.
    uint8_t last_8;
    // A count of the number of detected rollovers. Using this as bits 47-32
    // of the upper half of a 64-bit value results in a 48-bit tick counter.
    // This extends the total rollover period from about 49 days to about 8800
    // years while still allowing it to be stored with last_8 in a single
    // 32-bit value.
    uint16_t rollovers;
  } as_values;
};
subtle::Atomic32 g_last_time_and_rollovers = 0;
static_assert(
    sizeof(LastTimeAndRolloversState) <= sizeof(g_last_time_and_rollovers),
    "LastTimeAndRolloversState does not fit in a single atomic word");

// We use timeGetTime() to implement TimeTicks::Now().  This can be problematic
// because it returns the number of milliseconds since Windows has started,
// which will roll over the 32-bit value every ~49 days.  We try to track
// rollover ourselves, which works if TimeTicks::Now() is called at least every
// 48.8 days (not 49 days because only changes in the top 8 bits get noticed).
TimeTicks RolloverProtectedNow() {
  LastTimeAndRolloversState state;
  DWORD now;  // DWORD is always unsigned 32 bits.

  while (true) {
    // Fetch the "now" and "last" tick values, updating "last" with "now" and
    // incrementing the "rollovers" counter if the tick-value has wrapped back
    // around. Atomic operations ensure that both "last" and "rollovers" are
    // always updated together.
    int32_t original = subtle::Acquire_Load(&g_last_time_and_rollovers);
    state.as_opaque_32 = original;
    now = g_tick_function();
    uint8_t now_8 = static_cast<uint8_t>(now >> 24);
    if (now_8 < state.as_values.last_8)
      ++state.as_values.rollovers;
    state.as_values.last_8 = now_8;

    // If the state hasn't changed, exit the loop.
    if (state.as_opaque_32 == original)
      break;

    // Save the changed state. If the existing value is unchanged from the
    // original, exit the loop.
    int32_t check = subtle::Release_CompareAndSwap(
        &g_last_time_and_rollovers, original, state.as_opaque_32);
    if (check == original)
      break;

    // Another thread has done something in between so retry from the top.
  }

  return TimeTicks() +
         TimeDelta::FromMilliseconds(
             now + (static_cast<uint64_t>(state.as_values.rollovers) << 32));
}

// Discussion of tick counter options on Windows:
//
// (1) CPU cycle counter. (Retrieved via RDTSC)
// The CPU counter provides the highest resolution time stamp and is the least
// expensive to retrieve. However, on older CPUs, two issues can affect its
// reliability: First it is maintained per processor and not synchronized
// between processors. Also, the counters will change frequency due to thermal
// and power changes, and stop in some states.
//
// (2) QueryPerformanceCounter (QPC). The QPC counter provides a high-
// resolution (<1 microsecond) time stamp. On most hardware running today, it
// auto-detects and uses the constant-rate RDTSC counter to provide extremely
// efficient and reliable time stamps.
//
// On older CPUs where RDTSC is unreliable, it falls back to using more
// expensive (20X to 40X more costly) alternate clocks, such as HPET or the ACPI
// PM timer, and can involve system calls; and all this is up to the HAL (with
// some help from ACPI). According to
// http://blogs.msdn.com/oldnewthing/archive/2005/09/02/459952.aspx, in the
// worst case, it gets the counter from the rollover interrupt on the
// programmable interrupt timer. In best cases, the HAL may conclude that the
// RDTSC counter runs at a constant frequency, then it uses that instead. On
// multiprocessor machines, it will try to verify the values returned from
// RDTSC on each processor are consistent with each other, and apply a handful
// of workarounds for known buggy hardware. In other words, QPC is supposed to
// give consistent results on a multiprocessor computer, but for older CPUs it
// can be unreliable due bugs in BIOS or HAL.
//
// (3) System time. The system time provides a low-resolution (from ~1 to ~15.6
// milliseconds) time stamp but is comparatively less expensive to retrieve and
// more reliable. Time::EnableHighResolutionTimer() and
// Time::ActivateHighResolutionTimer() can be called to alter the resolution of
// this timer; and also other Windows applications can alter it, affecting this
// one.

using TimeTicksNowFunction = decltype(&TimeTicks::Now);
TimeTicks InitialNowFunction();

// See "threading notes" in InitializeNowFunctionPointer() for details on how
// concurrent reads/writes to these globals has been made safe.
TimeTicksNowFunction g_time_ticks_now_function = &InitialNowFunction;
int64_t g_qpc_ticks_per_second = 0;

TimeDelta QPCValueToTimeDelta(LONGLONG qpc_value) {
  // Ensure that the assignment to |g_qpc_ticks_per_second|, made in
  // InitializeNowFunctionPointer(), has happened by this point.
  std::atomic_thread_fence(std::memory_order_acquire);

  CR_DCHECK(g_qpc_ticks_per_second > 0);

  // If the QPC Value is below the overflow threshold, we proceed with
  // simple multiply and divide.
  if (qpc_value < Time::kQPCOverflowThreshold) {
    return TimeDelta::FromMicroseconds(
        qpc_value * Time::kMicrosecondsPerSecond / g_qpc_ticks_per_second);
  }
  // Otherwise, calculate microseconds in a round about manner to avoid
  // overflow and precision issues.
  int64_t whole_seconds = qpc_value / g_qpc_ticks_per_second;
  int64_t leftover_ticks = qpc_value - (whole_seconds * g_qpc_ticks_per_second);
  return TimeDelta::FromMicroseconds(
      (whole_seconds * Time::kMicrosecondsPerSecond) +
      ((leftover_ticks * Time::kMicrosecondsPerSecond) /
       g_qpc_ticks_per_second));
}

TimeTicks QPCNow() {
  return TimeTicks() + QPCValueToTimeDelta(QPCNowRaw());
}

void InitializeNowFunctionPointer() {
  LARGE_INTEGER ticks_per_sec = {};
  if (!QueryPerformanceFrequency(&ticks_per_sec))
    ticks_per_sec.QuadPart = 0;

  // If Windows cannot provide a QPC implementation, TimeTicks::Now() must use
  // the low-resolution clock.
  //
  // If the QPC implementation is expensive and/or unreliable, TimeTicks::Now()
  // will still use the low-resolution clock. A CPU lacking a non-stop time
  // counter will cause Windows to provide an alternate QPC implementation that
  // works, but is expensive to use.
  //
  // Otherwise, Now uses the high-resolution QPC clock. As of 21 August 2015,
  // ~72% of users fall within this category.
  CPUInfo cpu;
  const TimeTicksNowFunction now_function =
      (ticks_per_sec.QuadPart <= 0 || !cpu.has_non_stop_time_stamp_counter())
          ? &RolloverProtectedNow
          : &QPCNow;

  // Threading note 1: In an unlikely race condition, it's possible for two or
  // more threads to enter InitializeNowFunctionPointer() in parallel. This is
  // not a problem since all threads should end up writing out the same values
  // to the global variables.
  //
  // Threading note 2: A release fence is placed here to ensure, from the
  // perspective of other threads using the function pointers, that the
  // assignment to |g_qpc_ticks_per_second| happens before the function pointers
  // are changed.
  g_qpc_ticks_per_second = ticks_per_sec.QuadPart;
  std::atomic_thread_fence(std::memory_order_release);
 
  g_time_ticks_now_function = now_function;
}

TimeTicks InitialNowFunction() {
  InitializeNowFunctionPointer();
  return g_time_ticks_now_function();
}

}  // namespace

// static
TimeTicks::TickFunctionType TimeTicks::SetMockTickFunction(
    TickFunctionType ticker) {
  TickFunctionType old = g_tick_function;
  g_tick_function = ticker;
  subtle::NoBarrier_Store(&g_last_time_and_rollovers, 0);
  return old;
}

// static
TimeTicks TimeTicks::Now() {
  return g_time_ticks_now_function();
}

// static
bool TimeTicks::IsHighResolution() {
  if (g_time_ticks_now_function == &InitialNowFunction)
    InitializeNowFunctionPointer();
  return g_time_ticks_now_function == &QPCNow;
}

// static
bool TimeTicks::IsConsistentAcrossProcesses() {
  // According to Windows documentation [1] QPC is consistent post-Windows
  // Vista. So if we are using QPC then we are consistent which is the same as
  // being high resolution.
  //
  // [1] https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
  //
  // "In general, the performance counter results are consistent across all
  // processors in multi-core and multi-processor systems, even when measured on
  // different threads or processes. Here are some exceptions to this rule:
  // - Pre-Windows Vista operating systems that run on certain processors might
  // violate this consistency because of one of these reasons:
  //     1. The hardware processors have a non-invariant TSC and the BIOS
  //     doesn't indicate this condition correctly.
  //     2. The TSC synchronization algorithm that was used wasn't suitable for
  //     systems with large numbers of processors."
  return IsHighResolution();
}

// static
TimeTicks::Clock TimeTicks::GetClock() {
  return IsHighResolution() ? Clock::WIN_QPC
                            : Clock::WIN_ROLLOVER_PROTECTED_TIME_GET_TIME;
}

// static
TimeTicks TimeTicks::FromQPCValue(LONGLONG qpc_value) {
  return TimeTicks() + QPCValueToTimeDelta(qpc_value);
}

// TimeDelta ------------------------------------------------------------------

// static
TimeDelta TimeDelta::FromQPCValue(LONGLONG qpc_value) {
  return QPCValueToTimeDelta(qpc_value);
}

// static
TimeDelta TimeDelta::FromFileTime(FILETIME ft) {
  return TimeDelta::FromMicroseconds(FileTimeToMicroseconds(ft));
}

}  // namespace cr

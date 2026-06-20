// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/time/time.h"

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <limits>

#include "cr_base/compiler_config.h"

#include "cr_base/logging/logging.h"
#include "cr_base/numerics/safe_math.h"
#include "cr_base/memory/no_destructor.h"
#include "cr_base/synchronization/lock.h"

namespace {

// This prevents a crash on traversing the environment global and looking up
// the 'TZ' variable in libc. See: crbug.com/390567.
cr::Lock* GetSysTimeToTimeStructLock() {
  static cr::NoDestructor<cr::Lock> lock;
  return lock.get();
}

typedef time_t SysTime;

SysTime SysTimeFromTimeStruct(struct tm* timestruct, bool is_local) {
  cr::AutoLock locked(*GetSysTimeToTimeStructLock());
  return is_local ? mktime(timestruct) : timegm(timestruct);
}

void SysTimeToTimeStruct(SysTime t, struct tm* timestruct, bool is_local) {
  cr::AutoLock locked(*GetSysTimeToTimeStructLock());
  if (is_local)
    localtime_r(&t, timestruct);
  else
    gmtime_r(&t, timestruct);
}

int64_t ConvertTimespecToMicros(const struct timespec& ts) {
  // On 32-bit systems, the calculation cannot overflow int64_t.
  // 2**32 * 1000000 + 2**64 / 1000 < 2**63
  if (sizeof(ts.tv_sec) <= 4 && sizeof(ts.tv_nsec) <= 8) {
    int64_t result = ts.tv_sec;
    result *= cr::Time::kMicrosecondsPerSecond;
    result += (ts.tv_nsec / cr::Time::kNanosecondsPerMicrosecond);
    return result;
  }
  cr::CheckedNumeric<int64_t> result(ts.tv_sec);
  result *= cr::Time::kMicrosecondsPerSecond;
  result += (ts.tv_nsec / cr::Time::kNanosecondsPerMicrosecond);
  return result.ValueOrDie();
}

// Helper function to get results from clock_gettime() and convert to a
// microsecond timebase. Minimum requirement is MONOTONIC_CLOCK to be supported
// on the system. FreeBSD 6 has CLOCK_MONOTONIC but defines
// _POSIX_MONOTONIC_CLOCK to -1.
#if (defined(MINI_CHROMIUM_OS_POSIX) && defined(_POSIX_MONOTONIC_CLOCK) && \
     _POSIX_MONOTONIC_CLOCK >= 0) ||                         \
    defined(MINI_CHROMIUM_OS_BSD)
int64_t ClockNow(clockid_t clk_id) {
  struct timespec ts;
  CR_CHECK(clock_gettime(clk_id, &ts) == 0);
  return ConvertTimespecToMicros(ts);
}

cr::Optional<int64_t> MaybeClockNow(clockid_t clk_id) {
  struct timespec ts;
  int res = clock_gettime(clk_id, &ts);
  if (res == 0)
    return ConvertTimespecToMicros(ts);
  return cr::nullopt;
}

#else  // _POSIX_MONOTONIC_CLOCK
#error No usable tick clock function on this platform.
#endif  // _POSIX_MONOTONIC_CLOCK

}  // namespace

namespace cr {

// Time -----------------------------------------------------------------------

// static
bool Time::FromExploded(bool is_local, const Exploded& exploded, Time* time) {
  CheckedNumeric<int> month = exploded.month;
  month--;
  CheckedNumeric<int> year = exploded.year;
  year -= 1900;
  if (!month.IsValid() || !year.IsValid()) {
    *time = Time(0);
    return false;
  }

  struct tm timestruct;
  timestruct.tm_sec = exploded.second;
  timestruct.tm_min = exploded.minute;
  timestruct.tm_hour = exploded.hour;
  timestruct.tm_mday = exploded.day_of_month;
  timestruct.tm_mon = month.ValueOrDie();
  timestruct.tm_year = year.ValueOrDie();
  timestruct.tm_wday = exploded.day_of_week;  // mktime/timegm ignore this
  timestruct.tm_yday = 0;                     // mktime/timegm ignore this
  timestruct.tm_isdst = -1;                   // attempt to figure it out
  timestruct.tm_gmtoff = 0;   // not a POSIX field, so mktime/timegm ignore
  timestruct.tm_zone = nullptr;  // not a POSIX field, so mktime/timegm ignore

  SysTime seconds;

  // Certain exploded dates do not really exist due to daylight saving times,
  // and this causes mktime() to return implementation-defined values when
  // tm_isdst is set to -1. On Android, the function will return -1, while the
  // C libraries of other platforms typically return a liberally-chosen value.
  // Handling this requires the special code below.

  // SysTimeFromTimeStruct() modifies the input structure, save current value.
  struct tm timestruct0 = timestruct;

  seconds = SysTimeFromTimeStruct(&timestruct, is_local);
  if (seconds == -1) {
    // Get the time values with tm_isdst == 0 and 1, then select the closest one
    // to UTC 00:00:00 that isn't -1.
    timestruct = timestruct0;
    timestruct.tm_isdst = 0;
    int64_t seconds_isdst0 = SysTimeFromTimeStruct(&timestruct, is_local);

    timestruct = timestruct0;
    timestruct.tm_isdst = 1;
    int64_t seconds_isdst1 = SysTimeFromTimeStruct(&timestruct, is_local);

    // seconds_isdst0 or seconds_isdst1 can be -1 for some timezones.
    // E.g. "CLST" (Chile Summer Time) returns -1 for 'tm_isdt == 1'.
    if (seconds_isdst0 < 0)
      seconds = seconds_isdst1;
    else if (seconds_isdst1 < 0)
      seconds = seconds_isdst0;
    else
      seconds = std::min(seconds_isdst0, seconds_isdst1);
  }

  // Handle overflow.  Clamping the range to what mktime and timegm might
  // return is the best that can be done here.  It's not ideal, but it's better
  // than failing here or ignoring the overflow case and treating each time
  // overflow as one second prior to the epoch.
  int64_t milliseconds = 0;
  if (seconds == -1 && (exploded.year < 1969 || exploded.year > 1970)) {
    // If exploded.year is 1969 or 1970, take -1 as correct, with the
    // time indicating 1 second prior to the epoch.  (1970 is allowed to handle
    // time zone and DST offsets.)  Otherwise, return the most future or past
    // time representable.  Assumes the time_t epoch is 1970-01-01 00:00:00 UTC.
    //
    // The minimum and maximum representible times that mktime and timegm could
    // return are used here instead of values outside that range to allow for
    // proper round-tripping between exploded and counter-type time
    // representations in the presence of possible truncation to time_t by
    // division and use with other functions that accept time_t.
    //
    // When representing the most distant time in the future, add in an extra
    // 999ms to avoid the time being less than any other possible value that
    // this function can return.

    // On Android, SysTime is int64_t, special care must be taken to avoid
    // overflows.
    const int64_t min_seconds = (sizeof(SysTime) < sizeof(int64_t))
                                    ? std::numeric_limits<SysTime>::min()
                                    : std::numeric_limits<int32_t>::min();
    const int64_t max_seconds = (sizeof(SysTime) < sizeof(int64_t))
                                    ? std::numeric_limits<SysTime>::max()
                                    : std::numeric_limits<int32_t>::max();
    if (exploded.year < 1969) {
      milliseconds = min_seconds * kMillisecondsPerSecond;
    } else {
      milliseconds = max_seconds * kMillisecondsPerSecond;
      milliseconds += (kMillisecondsPerSecond - 1);
    }
  } else {
    CheckedNumeric<int64_t> checked_millis = seconds;
    checked_millis *= kMillisecondsPerSecond;
    checked_millis += exploded.millisecond;
    if (!checked_millis.IsValid()) {
      *time = Time(0);
      return false;
    }
    milliseconds = checked_millis.ValueOrDie();
  }

  Time converted_time;
  if (!FromMillisecondsSinceUnixEpoch(milliseconds, &converted_time)) {
    *time = cr::Time(0);
    return false;
  }

  // If |exploded.day_of_month| is set to 31 on a 28-30 day month, it will
  // return the first day of the next month. Thus round-trip the time and
  // compare the initial |exploded| with |utc_to_exploded| time.
  Time::Exploded to_exploded;
  if (!is_local)
    converted_time.UTCExplode(&to_exploded);
  else
    converted_time.LocalExplode(&to_exploded);

  if (ExplodedMostlyEquals(to_exploded, exploded)) {
    *time = converted_time;
    return true;
  }

  *time = Time(0);
  return false;
}

void Time::Explode(bool is_local, Exploded* exploded) const {
  // Time stores times with microsecond resolution, but Exploded only carries
  // millisecond resolution, so begin by being lossy.  Adjust from Windows
  // epoch (1601) to Unix epoch (1970);
  int64_t microseconds = us_ - kTimeTToMicrosecondsOffset;
  // The following values are all rounded towards -infinity.
  int64_t milliseconds;  // Milliseconds since epoch.
  SysTime seconds;       // Seconds since epoch.
  int millisecond;       // Exploded millisecond value (0-999).
  if (microseconds >= 0) {
    // Rounding towards -infinity <=> rounding towards 0, in this case.
    milliseconds = microseconds / kMicrosecondsPerMillisecond;
    seconds = milliseconds / kMillisecondsPerSecond;
    millisecond = milliseconds % kMillisecondsPerSecond;
  } else {
    // Round these *down* (towards -infinity).
    milliseconds = (microseconds - kMicrosecondsPerMillisecond + 1) /
                   kMicrosecondsPerMillisecond;
    seconds =
        (milliseconds - kMillisecondsPerSecond + 1) / kMillisecondsPerSecond;
    // Make this nonnegative (and between 0 and 999 inclusive).
    millisecond = milliseconds % kMillisecondsPerSecond;
    if (millisecond < 0)
      millisecond += kMillisecondsPerSecond;
  }

  struct tm timestruct;
  SysTimeToTimeStruct(seconds, &timestruct, is_local);

  exploded->year = timestruct.tm_year + 1900;
  exploded->month = timestruct.tm_mon + 1;
  exploded->day_of_week = timestruct.tm_wday;
  exploded->day_of_month = timestruct.tm_mday;
  exploded->hour = timestruct.tm_hour;
  exploded->minute = timestruct.tm_min;
  exploded->second = timestruct.tm_sec;
  exploded->millisecond = millisecond;
}


Time Time::Now() {
  struct timeval tv;
  struct timezone tz = {0, 0};  // UTC
  CR_CHECK(gettimeofday(&tv, &tz) == 0);
  // Combine seconds and microseconds in a 64-bit field containing microseconds
  // since the epoch.  That's enough for nearly 600 centuries.  Adjust from
  // Unix (1970) to Windows (1601) epoch.
  return Time() + TimeDelta::FromMicroseconds(
                      (tv.tv_sec * Time::kMicrosecondsPerSecond + tv.tv_usec) +
                      Time::kTimeTToMicrosecondsOffset);
}

Time Time::NowFromSystemTime() {
  // Just use TimeNowIgnoringOverride() because it returns the system time.
  return Time::Now();
}

// TimeTicks ------------------------------------------------------------------

TimeTicks TimeTicks::Now() {
  return TimeTicks() + TimeDelta::FromMicroseconds(ClockNow(CLOCK_MONOTONIC));
}

///cr::Optional<TimeTicks> MaybeTimeTicksNowIgnoringOverride() {
///  cr::Optional<int64_t> now = MaybeClockNow(CLOCK_MONOTONIC);
///  if (now.has_value())
///    return TimeTicks() + TimeDelta::FromMicroseconds(now.value());
///  return cr::nullopt;
///}

// static
TimeTicks::Clock TimeTicks::GetClock() {
  return Clock::LINUX_CLOCK_MONOTONIC;
}

// static
bool TimeTicks::IsHighResolution() {
  return true;
}

// static
bool TimeTicks::IsConsistentAcrossProcesses() {
  return true;
}

}  // namespace cr

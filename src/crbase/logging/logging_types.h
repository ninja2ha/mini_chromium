// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_TYPES_H_
#define MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_TYPES_H_

#include <stdint.h>

#include "crbuild/build_config.h"

namespace cr {
namespace logging {

// A bitmask of potential logging destinations.
using LoggingDestination = uint32_t;

// Specifies where logs will be written. Multiple destinations can be specified
// with bitwise OR.
// Unless destination is LOG_NONE, all logs with severity ERROR and above will
// be written to stderr in addition to the specified destination.
enum : uint32_t {
  LOG_TO_NULL = 0,
  LOG_TO_FILE = 1 << 0,
  LOG_TO_SYSTEM_DEBUG_LOG = 1 << 1,
  LOG_TO_STDERR = 1 << 2,
  LOG_TO_ALL = LOG_TO_FILE | LOG_TO_SYSTEM_DEBUG_LOG | LOG_TO_STDERR,

  // On Windows, use a file next to the exe.
  // On POSIX platforms, where it may not even be possible to locate the
  // executable on disk, use stderr.
#if defined(MINI_CHROMIUM_OS_WIN)
  LOG_TO_DEFAULT = LOG_TO_FILE,
#elif defined(MINI_CHROMIUM_OS_POSIX)
  LOG_TO_DEFAULT = LOG_TO_SYSTEM_DEBUG_LOG | LOG_TO_STDERR,
#endif
};

typedef int LogSeverity;

// This is level 1 verbosity
// Note: the log severities are used to index into the array of names,
// see log_severity_names.
constexpr LogSeverity kLogVerbose = -1;  
constexpr LogSeverity kLogInfo = 0;
constexpr LogSeverity kLogWarning = 1;
constexpr LogSeverity kLogError = 2;
constexpr LogSeverity kLogFatal = 3;
constexpr LogSeverity kLogNumSeverities = 4;

// kLogDFatal is kLogFatal in debug mode, Error in normal mode
#if defined(NDEBUG)
const LogSeverity kLogDFatal = kLogError;
#else
const LogSeverity kLogDFatal = kLogFatal;
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
typedef unsigned long SystemErrorCode;
#elif defined(MINI_CHROMIUM_OS_POSIX)
typedef int SystemErrorCode;
#endif

}  // namespace logging
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_TYPES_H_
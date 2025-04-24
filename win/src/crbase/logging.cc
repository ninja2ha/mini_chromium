// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(_MSC_VER) && !defined(_SCL_SECURE_NO_WARNINGS)
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "crbase/logging.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <time.h>
#include <errno.h>
#include <paths.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#endif

#include <algorithm>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <string>
#include <utility>

#include "crbase/helper/stl_util.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/debug/alias.h"
#include "crbase/debug/immediate_crash.h"
#include "crbase/strings/string_piece.h"

namespace cr {
namespace logging {

namespace {

const char* const log_severity_names[] = {"INFO", "WARNING", "ERROR", "FATAL"};
static_assert(kLogNumSeverities== cr::size(log_severity_names),
              "Incorrect number of log_severity_names");

uint64_t TickCount() {
#if defined(MINI_CHROMIUM_OS_WIN)
  return GetTickCount();
#elif defined(MINI_CHROMIUM_OS_POSIX)
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);

  uint64_t absolute_micro = static_cast<int64_t>(ts.tv_sec) * 1000000 +
                            static_cast<int64_t>(ts.tv_nsec) / 1000;

  return absolute_micro;
#endif
}

}  // namespace

LogMessage::LogMessage(const LoggingConfig& config,
                       const char* file, int line, LogSeverity severity)
    : config_(config),
      severity_(severity), file_(file), line_(line) {
  Init(file, line);
}

LogMessage::LogMessage(const LoggingConfig& config,
                       const char* file, int line, const char* condition) 
    : config_(config),
      severity_(kLogFatal), file_(file), line_(line) {
  Init(file, line);
  stream_ << "Check failed: " << condition << ". ";
}

// writes the common header info to the stream
void LogMessage::Init(const char* file, int line) {
  if (config_.fn_log_message_init != nullptr) 
    config_.fn_log_message_init(config_, severity_, file, line, stream_);
  message_start_ = stream_.str().length();
}

LogMessage::~LogMessage() {
  size_t stack_start = static_cast<size_t>(stream_.tellp());

  stream_ << std::endl;
  std::string str_newline(stream_.str());

  // Give any log message handler first dibs on the message.
  if (config_.fn_log_message_handler &&
      config_.fn_log_message_handler(config_ ,severity_, file_, line_,
                                     message_start_, str_newline)) {
    // The handler took care of it, no further processing.
    return;
  }

  if ((config_.logging_dest & kLogToSystemDebugLog) != 0) {
#if defined(MINI_CHROMIUM_OS_WIN)
    ::OutputDebugStringA(str_newline.c_str());
#endif
  }

  if ((config_.logging_dest & kLogToStdErr) != 0) {
    cr::ignore_result(
        fwrite(str_newline.data(), str_newline.size(), 1, stderr));
    fflush(stderr);
  }

  if ((config_.logging_dest & kLogToFile) != 0) {
    // We can have multiple threads and/or processes, so try to prevent them
    // from clobbering each other's writes.
    // If the client app did not call InitLogging, and the lock has not
    // been created do it now. We do this on demand, but if two threads try
    // to do this at the same time, there will be a race condition to create
    // the lock. This is why InitLogging should be called from the main
    // thread at the beginning of execution.
#if defined(MINI_CHROMIUM_OS_POSIX)
    LoggingLock::Init(kLockLogFile, nullptr);
    LoggingLock logging_lock;
#endif

    if (config_.log_file_handle != nullptr) {
#if defined(MINI_CHROMIUM_OS_WIN)
      DWORD num_written;
      ::WriteFile(config_.log_file_handle,
                  static_cast<const void*>(str_newline.c_str()),
                  static_cast<DWORD>(str_newline.length()),
                  &num_written,
                  nullptr);
#elif defined(MINI_CHROMIUM_OS_POSIX)
      ignore_result(fwrite(
          str_newline.data(), str_newline.size(), 1, g_log_file));
      fflush(configs_.log_file);
#else
#error Unsupported platform
#endif
    }
  }

  if (severity_ == kLogFatal) {
    // Ensure the first characters of the string are on the stack so they
    // are contained in minidumps for diagnostic purposes. We place start
    // and end marker values at either end, so we can scan captured stacks
    // for the data easily.
    struct {
      uint32_t start_marker = 0xbedead01;
      char data[1024];
      uint32_t end_marker = 0x5050dead;
    } str_stack;
    str_newline.copy(str_stack.data, cr::size(str_stack.data));
    cr::debug::Alias(&str_stack);

    if (config_.fn_log_assert_handler) {
      config_.fn_log_assert_handler(
        config_,
        file_, line_,
        cr::StringPiece(str_newline.c_str() + message_start_,
          stack_start - message_start_),
        cr::StringPiece(str_newline.c_str() + stack_start));
    }
    else {
      // Crash the process to generate a dump.
      CR_IMMEDIATE_CRASH();
    }
  }
}

SystemErrorCode GetLastSystemErrorCode() {
#if defined(MINI_CHROMIUM_OS_WIN)
  return ::GetLastError();
#elif defined(MINI_CHROMIUM_OS_POSIX)
  return errno;
#endif
}

#if defined(MINI_CHROMIUM_OS_WIN)
namespace {
std::string ErrorCodeToHex(SystemErrorCode error_code) {
  static_assert(sizeof(error_code) == 4, "");
  char str[] = "0x00000000";

  for (int i = 7; i >= 0; i--) {
    SystemErrorCode chr = error_code & 0xf;
    str[i + 2] = static_cast<char>(chr >= 10 ? (chr - 10) + 'A' : (chr + '0'));
    error_code >>= 4;
  }
  return str;
}
}  // namespace
#endif

std::string SystemErrorCodeToString(SystemErrorCode error_code) {
#if defined(MINI_CHROMIUM_OS_WIN)
  constexpr int kErrorMessageBufferSize = 256;
  char msgbuf[kErrorMessageBufferSize];
  DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD len = ::FormatMessageA(flags, nullptr, error_code, 0, msgbuf,
                               static_cast<DWORD>(cr::size(msgbuf)), nullptr);
  DWORD format_err = GetLastError();
  std::ostringstream ss;
  if (len) {
    // Messages returned by system end with line breaks. so remove it.
    while (--len) {
      if (msgbuf[len] == '\r' || msgbuf[len] == '\n') {
        msgbuf[len] = '\0';
        continue;
      }
      break;
    } 
    ss << msgbuf << ErrorCodeToHex(error_code);
    return ss.str();
  }
  ss << "Error (" << ErrorCodeToHex(format_err) << ") while retrieving error. ("
     << ErrorCodeToHex(error_code) << ")";
  return ss.str();

#elif defined(MINI_CHROMIUM_OS_POSIX)
  return base::safe_strerror(error_code) +
         base::StringPrintf(" (%d)", error_code);
#else
#error Not implemented
#endif  // defined(MINI_CHROMIUM_OS_WIN)
}

#if defined(MINI_CHROMIUM_OS_WIN)
Win32ErrorLogMessage::Win32ErrorLogMessage(const LoggingConfig& config, 
                                           const char* file,
                                           int line,
                                           LogSeverity severity,
                                           SystemErrorCode err)
    : err_(err),
      log_message_(config, file, line, severity) {
}

Win32ErrorLogMessage::~Win32ErrorLogMessage() {
  stream() << ": " << SystemErrorCodeToString(err_);
  // We're about to crash (CHECK). Put |err_| on the stack (by placing it in a
  // field) and use Alias in hopes that it makes it into crash dumps.
  DWORD last_error = err_;
  cr::debug::Alias(&last_error);
}

#elif defined(MINI_CHROMIUM_OS_POSIX)
ErrnoLogMessage::ErrnoLogMessage(const LoggingConfig& config, 
                                 const char* file,
                                 int line,
                                 LogSeverity severity,
                                 SystemErrorCode err)
    : err_(err),
      log_message_(configs, file, line, severity) {
}

ErrnoLogMessage::~ErrnoLogMessage() {
  stream() << ": " << SystemErrorCodeToString(err_);
  // We're about to crash (CHECK). Put |err_| on the stack (by placing it in a
  // field) and use Alias in hopes that it makes it into crash dumps.
  int last_error = err_;
  cr::debug::Alias(&last_error);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

const char* GetLogSeverityName(LogSeverity severity) {
  if (severity >= 0 && severity < kLogNumSeverities)
    return log_severity_names[severity];
  return "UNKNOWN";
}

void DefaultLogMessageInit(const LoggingConfig& config, 
                           LogSeverity severity,
                           const char* file, int line,
                           std::ostringstream& oss) {
  cr::StringPiece filename(file);
  size_t last_slash_pos = filename.find_last_of("\\/");
  if (last_slash_pos != cr::StringPiece::npos)
    filename.remove_prefix(last_slash_pos + 1);

  oss << "[";

  if (!config.prefix.empty())
    oss << config.prefix << ":";

#if defined(MINI_CHROMIUM_OS_WIN)
  SYSTEMTIME local_time;
  GetLocalTime(&local_time);
  oss << std::setfill('0')
      << std::setw(2) << local_time.wMonth
      << std::setw(2) << local_time.wDay
      << '/'
      << std::setw(2) << local_time.wHour
      << std::setw(2) << local_time.wMinute
      << std::setw(2) << local_time.wSecond
      << '.'
      << std::setw(3)
      << local_time.wMilliseconds
      << ':';
#elif defined(MINI_CHROMIUM_OS_POSIX)
  timeval tv;
  gettimeofday(&tv, nullptr);
  time_t t = tv.tv_sec;
  struct tm local_time;
  localtime_r(&t, &local_time);
  struct tm* tm_time = &local_time;
  oss << std::setfill('0')
      << std::setw(2) << 1 + tm_time->tm_mon
      << std::setw(2) << tm_time->tm_mday
      << '/'
      << std::setw(2) << tm_time->tm_hour
      << std::setw(2) << tm_time->tm_min
      << std::setw(2) << tm_time->tm_sec
      << '.'
      << std::setw(6) << tv.tv_usec
      << ':';
#else
#error Unsupported platform
#endif

  if (severity >= 0)
    oss << GetLogSeverityName(severity);
  else
    oss << "VERBOSE" << -severity;

  oss << ":" << filename << "(" << line << ")]";
}

bool ShouldCreateLogmessage(const LoggingConfig& config, LogSeverity severity) {
  if (config.logging_dest == kLogToFile && 
      config.log_file_handle == nullptr) {
    return false;
  }

  severity = severity >= kLogNumSeverities ? kLogFatal : severity;
  return config.min_severity <= severity;
}

LoggingConfig& GetDefaultLoggingConfig() {
  static cr::NoDestructor<LoggingConfig> default_logging_config;
  return *default_logging_config;
}

bool LoggingConfigInit(LoggingConfig& config) {
  if (config.verbose_level < kLogInfo)
    config.verbose_level = kLogInfo;

  if (config.min_severity < kLogInfo)
    config.min_severity = kLogInfo;
  else if (config.min_severity >= kLogNumSeverities)
    config.min_severity = kLogFatal;

  if ((config.logging_dest & kLogToFile) == 0) 
    return true;

#if defined(MINI_CHROMIUM_OS_WIN)
  LogFileHandle file_handle = ::CreateFileW(
      config.log_file, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
      nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file_handle == nullptr || file_handle == INVALID_HANDLE_VALUE) 
    return false;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  LogFileHandle file_handle = fopen(configs.log_file, "a");
  if (file_handle == nullptr)
    return false;
#else
#error Unsupported platform
#endif

  config.log_file_handle = file_handle;
  return true;
}

void LoggingConfigUninit(LoggingConfig& config) {
  if ((config.logging_dest & kLogToFile) == 0)
    return;

#if defined(MINI_CHROMIUM_OS_WIN)
  if (config.log_file_handle != nullptr) {
    ::CloseHandle(config.log_file_handle);
    config.log_file_handle = nullptr;
  }
#elif defined(MINI_CHROMIUM_OS_POSIX)
  if (config.log_file_handle != nullptr) {
    fclose(config.log_file_handle);
    config.log_file_handle = nullptr;
}
#else
#error Unsupported platform
#endif
}

std::ostream* g_swallow_stream;

}  // namespace logging
}  // namespace cr
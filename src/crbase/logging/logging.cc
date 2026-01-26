// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(_MSC_VER)

#if !defined(_SCL_SECURE_NO_WARNINGS)
#define _SCL_SECURE_NO_WARNINGS
#endif

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "crbase/logging/logging.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <string>
#include <utility>

#include "crbase/stl_util.h"
#include "crbase/debug/alias.h"
#include "crbase/memory/no_destructor.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/windows_types.h"
#include "crbase/logging/win/logging_internal_win.h"
#include <io.h>

#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <time.h>
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <errno.h>
#include <paths.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "crbase/logging/posix/logging_internal_posix.h"
#endif

namespace cr {
namespace logging {

namespace {

const char* const kLogSeverityNames[] = {"INFO", "WARNING", "ERROR", "FATAL"};
static_assert(kLogNumSeverities == cr::size(kLogSeverityNames),
              "Incorrect number of kLogSeverityNames!");

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

// We don't need locks on Windows for atomically appending to files. The OS
// provides this functionality.
#if defined(MINI_CHROMIUM_OS_POSIX)
// This class acts as a wrapper for locking the logging files.
// LoggingLocks can not be nested.
class LoggingLock {
 public:
  LoggingLock(void* log_mutex)
      : log_mutex_(reinterpret_cast<pthread_mutex_t*>(log_mutex)) {
    if (log_mutex_)
      pthread_mutex_lock(log_mutex_);
  }

  ~LoggingLock() {
    if (log_mutex_)
      pthread_mutex_unlock(log_mutex_);
  }

 private:
  // When we don't use a lock, we are using a global mutex. We need to do this
  // because LockFileEx is not thread safe.
  pthread_mutex_t* log_mutex_;
};
#endif


#if defined(MINI_CHROMIUM_OS_WIN)
std::wstring GetDefaultLogFile() {
  wchar_t run_dir[MAX_PATH];
  ZeroMemory(run_dir, cr::size(run_dir));
  ::GetModuleFileNameW(nullptr, run_dir, static_cast<DWORD>(cr::size(run_dir)));

  wchar_t* null_pos = wcsrchr(run_dir, L'\\');
  if (null_pos != nullptr)
    null_pos[1] = L'\0';

  SYSTEMTIME st;
  GetLocalTime(&st);

  // 2025-12-12 02-02-02.log
  wchar_t file_name[48] = {0};
  _snwprintf(file_name, cr::size(file_name),
             L"%04d-%02d-%02d %02d-%02d-%02d.log",
             st.wYear,
             st.wMonth,
             st.wDay,
             st.wHour,
             st.wMinute,
             st.wSecond);

  std::wstring log_gile = run_dir;
  log_gile += file_name;
  return log_gile;
}

#elif defined(MINI_CHROMIUM_OS_POSIX)
std::string GetDefaultLogFile() {
  timeval tv;
  gettimeofday(&tv, nullptr);
  time_t t = tv.tv_sec;
  struct tm local_time;
  localtime_r(&t, &local_time);
  struct tm* tm_time = &local_time;

  // 2025-12-12 02-02-02.log
  char file_name[48];
  snprintf(file_name, cr::size(file_name),
           "%04d-%02d-%02d %02d-%02d-%02d.log",
            tm_time->tm_year + 1900,
            1 + tm_time->tm_mon,
            tm_time->tm_mday,
            tm_time->tm_hour,
            tm_time->tm_min,
            tm_time->tm_sec);
  return file_name;
}
#endif

}  // namespace

LogMessage::LogMessage(const LoggingConfig& config,
                       const char* file,
                       int line,
                       LogSeverity severity)
    : config_(config), severity_(severity), file_(file), line_(line) {
  Init(file, line);
}

LogMessage::LogMessage(const LoggingConfig& config,
                       const char* file,
                       int line,
                       const char* condition)
    : config_(config), severity_(kLogFatal), file_(file), line_(line) {
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
      config_.fn_log_message_handler(config_,
                                     severity_,
                                     file_,
                                     line_,
                                     message_start_,
                                     str_newline)) {
    // The handler took care of it, no further processing.
    return;
  }

  if ((config_.logging_dest & LOG_TO_SYSTEM_DEBUG_LOG) != 0) {
#if defined(MINI_CHROMIUM_OS_WIN)
    ::OutputDebugStringA(str_newline.c_str());
#endif
  }

  if ((config_.logging_dest & LOG_TO_STDERR) != 0) {
    cr::ignore_result(
        fwrite(str_newline.data(), str_newline.size(), 1, stderr));
    fflush(stderr);
  }

  if ((config_.logging_dest & LOG_TO_FILE) != 0) {
    // We can have multiple threads and/or processes, so try to prevent them
    // from clobbering each other's writes.
    // If the client app did not call InitLogging, and the lock has not
    // been created do it now. We do this on demand, but if two threads try
    // to do this at the same time, there will be a race condition to create
    // the lock. This is why InitLogging should be called from the main
    // thread at the beginning of execution.
#if defined(MINI_CHROMIUM_OS_POSIX)
    LoggingLock logging_lock(config_.log_lock);
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
      cr::ignore_result(fwrite(str_newline.data(), str_newline.size(), 1,
                               config_.log_file_handle));
      fflush(config_.log_file_handle);
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
          file_,
          line_,
          str_newline.c_str() + message_start_,
          stack_start - message_start_,
          str_newline.c_str() + stack_start,
          str_newline.length() - stack_start);
    } else {
      // Crash the process to generate a dump.
      cr::debug::ImmediateCrash();
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

std::string SystemErrorCodeToString(SystemErrorCode error_code) {
#if defined(MINI_CHROMIUM_OS_WIN)
  return internal::strerror(error_code);
#elif defined(MINI_CHROMIUM_OS_POSIX)
  std::ostringstream ss;
  ss << internal::safe_strerror(error_code)
     << " (" << error_code << ")";
  return ss.str();
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
    : err_(err), log_message_(config, file, line, severity) {}

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
    : err_(err), log_message_(config, file, line, severity) {}

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
    return kLogSeverityNames[severity];
  return "UNKNOWN";
}

void DefaultLogMessageInit(const LoggingConfig& config,
                           LogSeverity severity,
                           const char* file,
                           int line,
                           std::ostringstream& oss) {
#if defined(MINI_CHROMIUM_OS_WIN)
  const char slash = '\\';
#elif defined(MINI_CHROMIUM_OS_POSIX)
  const char slash = '/';
#endif

  const char* file_name = file ? strrchr(file, slash) : "\0";
  if (file_name && file_name[0] != '\0') {
    file_name++;
  } else if (file_name == nullptr) {
    file_name = file;
  }

  oss << "[";

  if (config.prefix != nullptr)
    oss << config.prefix << ":";

#if defined(MINI_CHROMIUM_OS_WIN)
  SYSTEMTIME local_time;
  GetLocalTime(&local_time);
  oss << std::setfill('0')
      << std::setw(2) << local_time.wMonth
      << std::setw(2) << local_time.wDay << '/'
      << std::setw(2) << local_time.wHour
      << std::setw(2) << local_time.wMinute
      << std::setw(2) << local_time.wSecond << '.'
      << std::setw(3) << local_time.wMilliseconds
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
      << std::setw(2) << tm_time->tm_mday << '/'
      << std::setw(2) << tm_time->tm_hour
      << std::setw(2) << tm_time->tm_min
      << std::setw(2) << tm_time->tm_sec << '.'
      << std::setw(6) << tv.tv_usec
      << ':';
#else
#error Unsupported platform
#endif

  if (config.enable_tickcount)
    oss << TickCount() << ':';

  if (severity >= 0)
    oss << GetLogSeverityName(severity);
  else
    oss << "VERBOSE" << -severity;

  oss << ":" << file_name << "(" << line << ")]";
}

bool ShouldCreateLogmessage(const LoggingConfig& config, LogSeverity severity) {
  if (config.logging_dest == LOG_TO_FILE && config.log_file_handle == nullptr)
    return false;

  severity = severity >= kLogNumSeverities ? kLogFatal : severity;
  return config.min_severity <= severity;
}

bool InitializeConfig(LoggingConfig& config) {
  if (config.verbose_level < kLogInfo)
    config.verbose_level = kLogInfo;

  if (config.min_severity < kLogInfo)
    config.min_severity = kLogInfo;
  else if (config.min_severity >= kLogNumSeverities)
    config.min_severity = kLogFatal;

  if ((config.logging_dest & LOG_TO_FILE) == 0)
    return true;

  const PathChar* log_file = config.log_file;
  std::basic_string<PathChar> log_file_path;
  if (log_file == nullptr) {
    log_file_path = GetDefaultLogFile();
    log_file = log_file_path.c_str();
  }

#if defined(MINI_CHROMIUM_OS_WIN)
  LogFileHandle file_handle = ::CreateFileW(
      log_file,
      FILE_APPEND_DATA,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      nullptr,
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);
  if (file_handle == nullptr || file_handle == INVALID_HANDLE_VALUE)
    return false;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  LogFileHandle file_handle = fopen(log_file, "a");
  if (file_handle == nullptr)
    return false;
  if (config.log_lock == nullptr) {
    auto mutex = new pthread_mutex_t;
    *mutex = PTHREAD_MUTEX_INITIALIZER;
    config.log_lock  = mutex;
  }
#else
#error Unsupported platform
#endif

  config.log_file_handle = file_handle;
  return true;
}

void UninitializeConfig(LoggingConfig& config) {
  if ((config.logging_dest & LOG_TO_FILE) == 0)
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

  if (config.log_lock != nullptr) {
    delete reinterpret_cast<pthread_mutex_t*>(config.log_lock);
    config.log_lock = nullptr;
  }
#else
#error Unsupported platform
#endif
}

std::ostream* g_swallow_stream;

//------------------------------------------------------------------------------
// defualt logging config for Macros CR_LOG(xx).
LoggingConfig& GetDefaultConfig() {
  static cr::NoDestructor<LoggingConfig> default_logging_config;
  return *default_logging_config;
}

}  // namespace logging
}  // namespace cr
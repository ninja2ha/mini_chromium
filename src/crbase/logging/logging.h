// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_H_
#define MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_H_

#include <stddef.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "crbase/base_export.h"
#include "crbase/helper/scoped_clear_last_error.h"
#include "crbase/logging/logging_types.h"
#include "crbase/debug/immediate_crash.h"
#include "crbuild/compiler_specific.h"
#include "crbuild/build_config.h"

//
// Optional message capabilities
// -----------------------------
// Assertion failed messages and fatal errors are displayed in a dialog box
// before the application exits. However, running this UI creates a message
// loop, which causes application messages to be processed and potentially
// dispatched to existing application windows. Since the application is in a
// bad state when this assertion dialog is displayed, these messages may not
// get processed and hang the dialog, or the application might go crazy.
//
// Therefore, it can be beneficial to display the error dialog in a separate
// process from the main application. When the logging system needs to display
// a fatal error dialog box, it will look for a program called
// "DebugMessage.exe" in the same directory as the application executable. It
// will run this application with the message as the command line, and will
// not include the name of the application as is traditional for easier
// parsing.
//
// The code for DebugMessage.exe is only one line. In WinMain, do:
//   MessageBox(NULL, GetCommandLineW(), L"Fatal Error", 0);
//
// If DebugMessage.exe is not found, the logging code will use a normal
// MessageBox, potentially causing the problems discussed above.

// Instructions
// ------------
//
// Make a bunch of macros for logging.  The way to log things is to stream
// things to CR_LOG(<a particular severity level>).  E.g.,
//
//   CR_LOG(Info) << "Found " << num_cookies << " cookies";
//
// You can also do conditional logging:
//
//   CR_LOG_IF(Info, num_cookies > 10) << "Got lots of cookies";
//
// The CR_CHECK(condition) macro is active in both debug and release builds and
// effectively performs a CR_LOG(FATAL) which terminates the process and
// generates a crashdump unless a debugger is attached.
//
// There are also "debug mode" logging macros like the ones above:
//
//   CR_DLOG(Info) << "Found cookies";
//
//   CR_DLOG_IF(Info, num_cookies > 10) << "Got lots of cookies";
//
// All "debug mode" logging is compiled away to nothing for non-debug mode
// compiles.  LOG_IF and development flags also work well together
// because the code can be compiled away sometimes.
//
// We also have
//
//   CR_ASSERT(assertion);
//   CR_DASSERT(assertion);
//
// which is syntactic sugar for {,D}CRLOG_IF(FATAL, assert fails) << assertion;
//
// There are "verbose level" logging macros.  They look like
//
//   CR_VLOG(1) << "I'm printed when you run the program with --v=1 or more";
//   CR_VLOG(2) << "I'm printed when you run the program with --v=2 or more";
//
// These always log at the INFO log level (when they log at all).
// The verbose logging can also be turned on module-by-module.  For instance,
//    --vmodule=profile=2,icon_loader=1,browser_*=3,*/chromeos/*=4 --v=0
// will cause:
//   a. CR_VLOG(2) and lower messages to be printed from profile.{h,cc}
//   b. CR_VLOG(1) and lower messages to be printed from icon_loader.{h,cc}
//   c. CR_VLOG(3) and lower messages to be printed from files prefixed with
//      "browser"
//   d. CR_VLOG(4) and lower messages to be printed from files under a
//     "chromeos" directory.
//   e. CR_VLOG(0) and lower messages to be printed from elsewhere
//
// The wildcarding functionality shown by (c) supports both '*' (match
// 0 or more characters) and '?' (match any single character)
// wildcards.  Any pattern containing a forward or backward slash will
// be tested against the whole pathname and not just the module.
// E.g., "*/foo/bar/*=2" would change the logging level for all code
// in source files under a "foo/bar" directory.
//
// There's also CR_VLOG_IS_ON(n) "verbose level" condition macro. To be used as
//
//   if (CR_VLOG_IS_ON(2)) {
//     // do some logging preparation and logging
//     // that can't be accomplished with just CR_VLOG(2) << ...;
//   }
//
// There is also a CR_VLOG_IF "verbose level" condition macro for sample
// cases, when some extra computation and preparation for logs is not
// needed.
//
//   CR_VLOG_IF(1, (size > 1024))
//      << "I'm printed when size is more than 1024 and when you run the "
//         "program with --v=1 or more";
//
// We also override the standard 'assert' to use 'CR_DASSERT'.
//
// Lastly, there is:
//
//   CR_PLOG(Error) << "Couldn't do foo";
//   CR_DPLOG(Error) << "Couldn't do foo";
//   CR_PLOG_IF(Error, cond) << "Couldn't do foo";
//   CR_DPLOG_IF(Error, cond) << "Couldn't do foo";
//   CR_PCHECK(condition) << "Couldn't do foo";
//   CR_DPCHECK(condition) << "Couldn't do foo";
//
// which append the last system error to the message in string form (taken from
// GetLastError() on Windows and errno on POSIX).
//
// The supported severity levels for macros that allow you to specify one
// are (in increasing order of severity) Info, Warning, Error, and Fatal.
//
// Very important: logging a message at the FATAL severity level causes
// the program to terminate (after the message is logged).
//
// There is the special severity of DFATAL, which logs FATAL in debug mode,
// ERROR in normal mode.
//
// Output is of the format, for example:
// [0812/234555.406952:VERBOSE1:drm_device_handle.cc(90)] Succeeded
// authenticating /dev/dri/card0 in 0 ms with 1 attempt(s)
//
// The colon separated fields inside the brackets are as follows:
// 0. An optional Logfile prefix (not included in this example)
// 3. The date/time of the log message, in MMDD/HHMMSS.Milliseconds format
// 4. The log level
// 5. The filename and line number where the log was instantiated
//

namespace cr {
namespace logging {

// TODO(avi): do we want to do a unification of character types here?
#if defined(MINI_CHROMIUM_OS_WIN)
typedef wchar_t PathChar;
typedef void* LogFileHandle;
#else
typedef char PathChar;
typedef FILE* LogFileHandle;
#endif

struct LoggingConfig;
CRBASE_EXPORT const char* GetLogSeverityName(LogSeverity severity);
CRBASE_EXPORT void DefaultLogMessageInit(const LoggingConfig& config, 
                                         LogSeverity severity,
                                         const char* file, int line,
                                         std::ostringstream& oss);

struct CRBASE_EXPORT LoggingConfig {
  // [in]
  // Equivalent to logging destination enum, but allows for multiple
  // destinations.
#if defined(NDEBUG)
  uint32_t logging_dest = LOG_TO_NULL;
#else
  uint32_t logging_dest = LOG_TO_STDERR;
#endif

  // [in]
  // Logging file path. 
  // The default setting for this is "year-mon-day hh-mm-ss.log"
  const PathChar* log_file = nullptr;

  // [out]
  // The file that logs should be written to
  LogFileHandle log_file_handle = nullptr;
  
  // [in]
  const char* prefix = nullptr;

  // [in]
  LogSeverity min_severity = kLogInfo;

  // [in && out]
  // Do not setting negative numbers
  int verbose_level = 999;

  // [in]
  bool enable_tickcount = false;

  // [in]
  // The Function for writing log prefix.
  void(*fn_log_message_init)(const LoggingConfig& config,
                             LogSeverity severity, 
                             const char* file, 
                             int line, 
                             std::ostringstream& oss)
      = DefaultLogMessageInit;

  // [in]
  // Sets the Log Message Handler that gets passed every log message before
  // it's sent to other log destinations (if any).
  // Returns true to signal that it handled the message and the message
  // should not be sent to other log destinations.
  bool(*fn_log_message_handler)(const LoggingConfig& config, 
                                LogSeverity severity, 
                                const char* file, 
                                int line, 
                                size_t message_start, 
                                const std::string& message)
      = nullptr;

  // [in]
  // Sets the Log Assert Handler that will be used to notify of check failures.
  // The default handler shows a dialog box and then terminate the process,
  // however clients can use this function to override with their own handling
  // (e.g. a silent one for Unit Tests)
  void(*fn_log_assert_handler)(const LoggingConfig& config, 
                               const char* file, int line,
                               const char* message,
                               size_t message_len,
                               const char* stack_trace,
                               size_t stack_trace_len)
      = nullptr;

  #if defined(MINI_CHROMIUM_OS_POSIX)
  // [readonly]
  void* log_lock = nullptr;
#endif
};

// This class more or less represents a particular log message.  You
// create an instance of LogMessage and then stream stuff to it.
// When you finish streaming to it, ~LogMessage is called and the
// full message gets streamed to the appropriate destination.
//
// You shouldn't actually use LogMessage's constructor to log things,
// though.  You should use the LOG() macro (and variants thereof)
// above.
class CRBASE_EXPORT LogMessage {
 public:
  LogMessage(const LogMessage&) = delete;
  LogMessage& operator=(const LogMessage&) = delete;

  // Used for CR_LOG(severity).
  LogMessage(const LoggingConfig& conifg,
             const char* file, int line, LogSeverity severity);

  // Used for CR_CHECK().  Implied severity = LOG_FATAL.
  LogMessage(const LoggingConfig& conifg,
             const char* file, int line, const char* condition);

  ~LogMessage();

  std::ostream& stream() { return stream_; }

  LogSeverity severity() { return severity_; }
  std::string str() { return stream_.str(); }

 private:
  void Init(const char* file, int line);

  const LoggingConfig& config_;

  LogSeverity severity_;
  std::ostringstream stream_;
  size_t message_start_;  // Offset of the start of the message (past prefix
                          // info).
  // The file and line information passed in to the constructor.
  const char* file_;
  const int line_;
  ///const char* file_basename_;

  // This is useful since the LogMessage class uses a lot of Win32 calls
  // that will lose the value of GLE and the code that called the log function
  // will have lost the thread error value when the log call returns.
  cr::ScopedClearLastError last_error_;
};

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
public:
  LogMessageVoidify() = default;
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream&) { }
};

// Alias for ::GetLastError() on Windows and errno on POSIX. Avoids having to
// pull in windows.h just for GetLastError() and DWORD.
CRBASE_EXPORT SystemErrorCode GetLastSystemErrorCode();
CRBASE_EXPORT std::string SystemErrorCodeToString(SystemErrorCode error_code);

#if defined(MINI_CHROMIUM_OS_WIN)
// Appends a formatted system message of the GetLastError() type.
class CRBASE_EXPORT Win32ErrorLogMessage {
 public:
  Win32ErrorLogMessage(const Win32ErrorLogMessage&) = delete;
  Win32ErrorLogMessage& operator=(const Win32ErrorLogMessage&) = delete;

  Win32ErrorLogMessage(const LoggingConfig& config,
                       const char* file,
                       int line,
                       LogSeverity severity,
                       SystemErrorCode err);

  // Appends the error message before destructing the encapsulated class.
  ~Win32ErrorLogMessage();

  std::ostream& stream() { return log_message_.stream(); }

 private:
  SystemErrorCode err_;
  LogMessage log_message_;
};
#elif defined(MINI_CHROMIUM_OS_POSIX)
// Appends a formatted system message of the errno type
class CRBASE_EXPORT ErrnoLogMessage {
 public:
  ErrnoLogMessage(const ErrnoLogMessage&) = delete;
  ErrnoLogMessage& operator=(const ErrnoLogMessage&) = delete;

  ErrnoLogMessage(const LoggingConfig& config, 
                  const char* file,
                  int line,
                  LogSeverity severity,
                  SystemErrorCode err);

  // Appends the error message before destructing the encapsulated class.
  ~ErrnoLogMessage();

  std::ostream& stream() { return log_message_.stream(); }

 private:
  SystemErrorCode err_;
  LogMessage log_message_;
};
#endif

// Helper macro which avoids evaluating the arguments to a stream if
// the condition doesn't hold. Condition is evaluated once and only once.
#define CR_LAZY_STREAM(stream, condition)                                      \
  !(condition) ? (void) 0 : ::cr::logging::LogMessageVoidify() & (stream)

#define CR_CAN_LOG(config, severity)                                           \
  ::cr::logging::ShouldCreateLogmessage(config,                                \
                                       ::cr::logging::kLog##severity)

#define CR_CAN_VLOG(config, verboselevel)                                      \
  ((verboselevel) >= (config).verbose_level)

CRBASE_EXPORT bool ShouldCreateLogmessage(const LoggingConfig& config, 
                                          LogSeverity severity);

CRBASE_EXPORT bool InitializeConfig(LoggingConfig& config);
CRBASE_EXPORT void UninitializeConfig(LoggingConfig& config);

// A few definitions of macros that don't generate much code. These are used by
// CR_LOG() and CR_LOG_IF, etc. Since these are used all over our code, it's
// better to have compact code for these operations.
#define COMPACT_LOG_Info(ClassName, Config, File, Line, ...)              \
  ::cr::logging::ClassName(Config, File, Line, ::cr::logging::kLogInfo,   \
                           ##__VA_ARGS__)

#define COMPACT_LOG_Warning(ClassName, Config, File, Line, ...)           \
  ::cr::logging::ClassName(Config, File, Line, ::cr::logging::kLogWarning,\
                           ##__VA_ARGS__)

#define COMPACT_LOG_Error(ClassName, Config, File, Line, ...)             \
  ::cr::logging::ClassName(Config, File, Line, ::cr::logging::kLogError,  \
                           ##__VA_ARGS__)

#define COMPACT_LOG_Fatal(ClassName, Config, File, Line, ...)             \
  ::cr::logging::ClassName(Config, File, Line, ::cr::logging::kLogFatal,  \
                           ##__VA_ARGS__)

CRBASE_EXPORT extern std::ostream* g_swallow_stream;

// Note that g_swallow_stream is used instead of an arbitrary LOG() stream to
// avoid the creation of an object with a non-trivial destructor (LogMessage).
// On MSVC x86 (checked on 2015 Update 3), this causes a few additional
// pointless instructions to be emitted even at full optimization level, even
// though the : arm of the ternary operator is clearly never executed. Using a
// simpler object to be &'d with Voidify() avoids these extra instructions.
// Using a simpler POD object with a templated operator<< also works to avoid
// these instructions. However, this causes warnings on statically defined
// implementations of operator<<(std::ostream, ...) in some .cc files, because
// they become defined-but-unreferenced functions. A reinterpret_cast of 0 to an
// ostream* also is not suitable, because some compilers warn of undefined
// behavior.
#define CR_EAT_STREAM_PARAMETERS(ignored)   \
  true || (ignored) ? (void)0               \
       : ::cr::logging::LogMessageVoidify() & (*::cr::logging::g_swallow_stream)

#if defined(__FILE_NAME__)
#define __CR_FILE__ __FILE_NAME__
#else
#define __CR_FILE__ __FILE__
#endif

//  -- cr logging: start -------------------------------------------------------

CRBASE_EXPORT LoggingConfig& GetDefaultConfig();

#define CR_DEFAULT_LOGGING_CONFIG ::cr::logging::GetDefaultConfig()

#define CR_LOG_STREAM(severity)                                                \
  COMPACT_LOG_##severity(LogMessage, CR_DEFAULT_LOGGING_CONFIG, __CR_FILE__,   \
                         __LINE__).stream()

#define CR_LOG(severity)                                                       \
  CR_LAZY_STREAM(CR_LOG_STREAM(severity),                                      \
                 CR_CAN_LOG(CR_DEFAULT_LOGGING_CONFIG, severity))

#define CR_LOG_IF(severity, condition)                                         \
  CR_LAZY_STREAM(CR_LOG_STREAM(severity),                                      \
                 CR_CAN_LOG(CR_DEFAULT_LOGGING_CONFIG,severity) && (condition))

// cr plog

#if defined(MINI_CHROMIUM_OS_WIN)
#define CR_PLOG_STREAM(severity)                                               \
  COMPACT_LOG_##severity(Win32ErrorLogMessage,                                 \
                         CR_DEFAULT_LOGGING_CONFIG,                            \
                         __CR_FILE__, __LINE__,                                \
                         ::cr::logging::GetLastSystemErrorCode()).stream()
#elif defined(MINI_CHROMIUM_OS_POSIX)
#define CR_PLOG_STREAM(severity)                                               \
  COMPACT_LOG_##severity(ErrnoLogMessage,                                      \
                         CR_DEFAULT_LOGGING_CONFIG,                            \
                         __FILE__, __LINE__,                                   \
                         ::cr::logging::GetLastSystemErrorCode()).stream()
#else
#error Unsupport Platform
#endif

#define CR_PLOG(severity)                                                      \
  CR_LAZY_STREAM(CR_PLOG_STREAM(severity),                                     \
                 CR_CAN_LOG(CR_DEFAULT_LOGGING_CONFIG,severity))

#define CR_PLOG_IF(severity, condition)                                        \
  CR_LAZY_STREAM(CR_PLOG_STREAM(severity),                                     \
                 CR_CAN_LOG(CR_DEFAULT_LOGGING_CONFIG,severity) && (condition))

// cr check

#ifdef NDEBUG
// Make all CR_CHECK functions discard their log strings to reduce code bloat, and
// improve performance, for official release builds.
//
// This is not calling BreakDebugger since this is called frequently, and
// calling an out-of-line function instead of a noreturn inline macro prevents
// compiler optimizations.
#define CR_CHECK(condition)                                                    \
  CR_UNLIKELY(!(condition)) ?                                                  \
      cr::debug::ImmediateCrash() : CR_EAT_STREAM_PARAMETERS(condition)

// PCHECK includes the system error code, which is useful for determining
// why the condition failed. In official builds, preserve only the error code
// message so that it is available in crash reports. The stringified
// condition and any additional stream parameters are dropped.
#define CR_PCHECK(condition)                                                   \
  CR_LAZY_STREAM(CR_PLOG_STREAM(Fatal), CR_UNLIKELY(!(condition)));            \
  CR_EAT_STREAM_PARAMETERS(condition)
#else
// Do as much work as possible out of line to reduce inline code size.
#define CR_CHECK(condition)                                                    \
  CR_LAZY_STREAM(::cr::logging::LogMessage(CR_DEFAULT_LOGGING_CONFIG,          \
                                           __CR_FILE__, __LINE__,              \
                                           #condition).stream(),               \
                 !(condition))

#define CR_PCHECK(condition)                                                   \
  CR_LAZY_STREAM(CR_PLOG_STREAM(Fatal), !(condition))                          \
      << "Check failed: " #condition ". "
#endif  // NDEBUG

// cr vlog
// The VLOG macros log with negative verbosities.
#define CR_VLOG_STREAM(verboselevel)                                           \
  ::cr::logging::LogMessage(CR_DEFAULT_LOGGING_CONFIG, __CR_FILE__,            \
                            __LINE__, -verboselevel).stream()

#define CR_VLOG_IS_ON(verboselevel)                                            \
  CR_CAN_VLOG(CR_DEFAULT_LOGGING_CONFIG, verboselevel)

#define CR_VLOG(verboselevel)                                                  \
  CR_LAZY_STREAM(CR_VLOG_STREAM(verboselevel), CR_VLOG_IS_ON(verboselevel))

#define CR_VLOG_IF(verboselevel, condition)                                    \
  CR_LAZY_STREAM(CR_VLOG_STREAM(verboselevel),                                 \
                 CR_VLOG_IS_ON(verboselevel) && (condition))

// CR_DLOG on ReleaseMode
#ifdef NDEBUG
#define CR_DLOG_IS_ON() false
#define CR_DCHECK_IS_ON() false

#define CR_DLOG(severity) CR_EAT_STREAM_PARAMETERS(true)
#define CR_DLOG_IF(severity, condition) CR_EAT_STREAM_PARAMETERS(condition)

#define CR_DPLOG(severity) CR_EAT_STREAM_PARAMETERS(true)
#define CR_DPLOG_IF(severity, condition) CR_EAT_STREAM_PARAMETERS(condition)

#define CR_DVLOG(verboselevel) CR_EAT_STREAM_PARAMETERS(verboselevel)
#define CR_DVLOG_IF(verboselevel, condition)           \
    CR_EAT_STREAM_PARAMETERS(verboselevel || condition)

#define CR_DCHECK(condition) CR_EAT_STREAM_PARAMETERS(condition)
#define CR_DPCHECK(condition) CR_EAT_STREAM_PARAMETERS(condition)
#else
#define CR_DLOG_IS_ON() true
#define CR_DCHECK_IS_ON() true

#define CR_DLOG(severity) CR_LOG(severity)
#define CR_DLOG_IF(severity, condition) CR_LOG_IF(severity, condition)

#define CR_DPLOG(severity) CR_PLOG(severity)
#define CR_DPLOG_IF(severity, condition)  CR_PLOG_IF(severity, condition)

#define CR_DVLOG(verboselevel) CR_VLOG(verboselevel)
#define CR_DVLOG_IF(verboselevel, condition) CR_VLOG_IF(verboselevel, condition)

#define CR_DCHECK(condition) CR_CHECK(condition)
#define CR_DPCHECK(condition) CR_PCHECK(condition)
#endif

// cr assert
#define CR_ASSERT(assertion) \
  CR_LOG_IF(Fatal, !(assertion))  << "Assert failed: " #assertion ". "

#define CR_DASSERT(assertion) \
  CR_DLOG_IF(Fatal, !(assertion))  << "Assert failed: " #assertion ". "

#define CR_NOTREACHED() CR_DLOG(Warning) << "NOT REACHED. "
#define CR_NOTIMPLEMENTED() CR_DLOG(Error) << "NOT IMPLEMENTED. "
#define CR_NOTIMPLEMENTED_LOG_ONCE()                                           \
  do {                                                                         \
    static bool logged_once = false;                                           \
    CR_DLOG_IF(Error, !logged_once) << "NOT IMPLEMENTED. ";                    \
    logged_once = true;                                                        \
  } while (0);                                                                 \
  CR_EAT_STREAM_PARAMETERS(true)

//  -- cr logging: end ---------------------------------------------------------

}  // namespace logging
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_H_
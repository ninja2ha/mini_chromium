#include <iostream>
#include <string>

#include "crbase/memory/no_destructor.h"
#include "crbase/logging.h"
#include "crbase/build_platform.h"

using namespace std;

//------------------------------------------------------------------------------

namespace {

void InitializeDefaultLogging() {
  auto &config = cr::logging::GetDefaultLoggingConfig();
  config.logging_dest = cr::logging::kLogToStdErr;
  config.verbose_level = 5;
  cr::logging::LoggingConfigInit(config);
}

// -- custom logging : start --

#define CUSTOM_LOGGING_CONFIG ::GetCustomLoggingConfig()

cr::logging::LoggingConfig& GetCustomLoggingConfig() {
  static cr::NoDestructor<cr::logging::LoggingConfig> custom_config;
  return *custom_config;
}

#define CS_LOG_STREAM(severity) \
  COMPACT_LOG_##severity(LogMessage, CUSTOM_LOGGING_CONFIG, nullptr, 0).stream()

#define CS_LOG(severity)                                                        \
  CR_LAZY_STREAM(                                                               \
      CS_LOG_STREAM(severity),                                                  \
      CR_CAN_LOG(CUSTOM_LOGGING_CONFIG, severity))

void InitializeCustomLogging() {
  auto &config = GetCustomLoggingConfig();
  config.logging_dest = cr::logging::kLogToFile; // enable config.log_file
  config.min_severity = cr::logging::kLogInfo;
  config.fn_log_message_init = nullptr;          // disable default prefix
#if defined(MINI_CHROMIUM_OS_WIN)
  config.log_file = L"debug.log";
#else
  config.log_file = "debug.log";
#endif
  cr::logging::LoggingConfigInit(config);
}

// -- custom logging : end --

}  // namespace

//------------------------------------------------------------------------------

int main(int argc, char* argv) {
  // mini_chromium default logging macros.
  InitializeDefaultLogging();
  CR_LOG(Info) << "default infomation";
  CR_LOG(Warning) << "default Warning";
  CR_LOG(Error) << "default Error";
  CR_VLOG(0) << "this log wasn`t not printed";
  CR_VLOG(6) << "print vlog";
  CR_CHECK(true) << "1111";

  // custom logging macros.
  InitializeCustomLogging();
  CS_LOG(Info) << "custom info";
  CS_LOG(Warning) << "custom Warning";
  CS_LOG(Error) << "custom error";

  getchar();
  return 0;
}

//------------------------------------------------------------------------------

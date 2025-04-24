// Copyright (c) 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_EXAMPLES_COMMON_LOGGING_INITIALIZTION_H_
#define MINI_CHROMIUM_SRC_EXAMPLES_COMMON_LOGGING_INITIALIZTION_H_

#include "crbase/logging.h"

namespace example {

inline void InitLogging() {
  cr::logging::LoggingConfig& config = cr::logging::GetDefaultLoggingConfig();
  config.logging_dest = cr::logging::kLogToStdErr;
  config.verbose_level = 0;
  cr::logging::LoggingConfigInit(config);
}

}  // namespace example

#endif  // MINI_CHROMIUM_SRC_EXAMPLES_COMMON_LOGGING_INITIALIZTION_H_
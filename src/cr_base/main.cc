///typedef struct IUnknown IUnknown;
///#include <windows.h>

#include <iostream>
#include <string>

#include "cr_base/compiler_specific.h"

#include "cr_base/byte_order.h"

#include "cr_base/logging/logging.h"
#include "cr_base/logging/logging_strerror.h"

#include "cr_base/util/scoped_clear_last_error.h"

int main() {
  auto& config = CR_DEFAULT_LOGGING_CONFIG;
  config.logging_dest = cr::logging::LOG_TO_STDERR;
  config.verbose_lowest_level = 999;
  config.prefix = "MyApp";
  cr::logging::InitializeConfig(config);

  CR_VLOG(1) << "1234561111";
  CR_VLOG(998) << "123456";

  CR_LOG(Info) << "hello world";

  return 0;
}
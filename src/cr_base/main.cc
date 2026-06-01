typedef struct IUnknown IUnknown;
#include <windows.h>

#include <iostream>
#include <string>

#include "cr_base/compiler_specific.h"

#include "cr_base/byte_order.h"

#include "cr_base/logging/logging.h"
#include "cr_base/logging/logging_strerror.h"

int main() {
 ///std::wstring str = cr::logging::StrErrorW(ERROR_ABANDON_HIBERFILE);
  CR_DEFAULT_LOGGING_CONFIG.logging_dest = cr::logging::LOG_TO_STDERR; 
  CR_DEFAULT_LOGGING_CONFIG.verbose_lowest_level = 999;
  cr::logging::InitializeConfig(CR_DEFAULT_LOGGING_CONFIG);

  CR_VLOG(1) << "1234561111";
  CR_VLOG(998) << "123456";

  CR_LOG(Info) << "hello world";

  return 0;
}
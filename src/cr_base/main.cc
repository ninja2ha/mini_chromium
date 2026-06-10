#include "cr_base/logging/logging.h"
#include "cr_base/numerics/safe_math.h"

int main() {
  auto& config = CR_DEFAULT_LOGGING_CONFIG;
  config.logging_dest = cr::logging::LOG_TO_STDERR /*| cr::logging::LOG_TO_FILE*/;
  config.verbose_lowest_level = 999;
  cr::logging::InitializeConfig(config);

  ///cr::CheckedNumeric<int> num = cr::CheckMax(10, 1, std::numeric_limits<uint32_t>::max());
  
  cr::ClampedNumeric<unsigned> a = cr::ClampSub(0u, 1);
  CR_LOG(Info) << "num = " << a;
  return 0;
}
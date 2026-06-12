#include "cr_base/logging/logging.h"
#include "cr_base/numerics/safe_math.h"
#include "cr_base/numerics/safe_conversions.h"

int main() {
  auto& config = CR_DEFAULT_LOGGING_CONFIG;
  config.logging_dest = cr::logging::LOG_TO_STDERR /*| cr::logging::LOG_TO_FILE*/;
  config.verbose_lowest_level = 999;
  cr::logging::InitializeConfig(config);

  ///cr::CheckedNumeric<int> num = -1.1f;
  cr::ClampedNumeric<unsigned> sum = cr::ClampAdd(4294967295u, 1, -1); // sum = 4294967295;
  //int a  = cr::StrictCast<int>(0x80000000u);
  unsigned b  = cr::StrictCast<unsigned>((char)1);

  //CR_LOG(Info) << "num = " << a;
  CR_LOG(Info) << "num = " << b;
  return 0;
}
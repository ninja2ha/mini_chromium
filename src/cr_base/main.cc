#include "cr_base/logging/logging.h"
#include "cr_base/numerics/safe_math.h"
#include "cr_base/numerics/safe_conversions.h"
#include "cr_base/strings/string_number_conversions.h"
#include "cr_base/time/time.h"

#include <windows.h>

#pragma comment(lib, "winmm")

int main() {
  auto& config = CR_DEFAULT_LOGGING_CONFIG;
  config.logging_dest = cr::logging::LOG_TO_STDERR /*| cr::logging::LOG_TO_FILE*/;
  config.verbose_lowest_level = 999;
  cr::logging::InitializeConfig(config);

  int32_t value = 0;
  bool br = cr::HexStringToInt("\\56", &value);
  CR_LOG(Info) << br << "_" << value;

  std::string str = cr::HexEncode(std::initializer_list<uint8_t>({0x90, 0x91, 0x92}));
  CR_LOG(Info) << str;

  CR_LOG(Info) << cr::Time::Now().ToLogString();
  CR_LOG(Info) << cr::TimeTicks::Now();
  ::Sleep(500);
  CR_LOG(Info) << cr::Time::NowFromSystemTime();
  CR_LOG(Info) << cr::TimeTicks::Now();

  str.clear();
  br = cr::HexStringToString("90 91 92", &str);
  CR_LOG(Info) << br;
  return 0;
}
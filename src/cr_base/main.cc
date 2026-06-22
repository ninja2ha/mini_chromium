///#ifndef NOMINMAX
///#define NOMINMAX
///#endif
///typedef struct IUnknown IUnknown;
///#include <windows.h>
#pragma comment(lib, "winmm")
#pragma comment(lib, "shlwapi")

#include "cr_base/logging/logging.h"
#include "cr_base/debug/alias.h"
#include "cr_base/numerics/safe_math.h"
#include "cr_base/numerics/safe_conversions.h"
#include "cr_base/strings/string_number_conversions.h"
#include "cr_base/strings/utf_string_conversions.h"
#include "cr_base/i18n/case_conversion.h"
#include "cr_base/time/time.h"
#include "cr_base/byte_size.h"

#include "cr_base/files/file_util.h"

int main() {
  auto& config = CR_DEFAULT_LOGGING_CONFIG;
  config.logging_dest = cr::logging::LOG_TO_STDERR;
  config.verbose_lowest_level = 999;
  cr::logging::InitializeConfig(config);

  constexpr cr::ByteSize kBufferSize = cr::MiBU(1u);
  constexpr cr::ByteSizeDelta kBufferSize2 = cr::MiBS(1);
  CR_LOG(Info) << kBufferSize.InBytes();
  CR_LOG(Info) << kBufferSize2.InBytes();

  cr::FilePath cur_dir;
  cr::GetCurrentDirectory(&cur_dir);
  cur_dir = cur_dir.Append(L"loglog.txt");

  cr::FilePath tmp_dir = cr::GetUniquePath(cur_dir);
  cr::WriteFile(tmp_dir, "123456");
  CR_LOG(Info) << tmp_dir;

  system("pause");
  return 0;
}
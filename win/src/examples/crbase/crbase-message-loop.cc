#include <iostream>
#include <string>

#include "crbase/logging.h"
#include "crbase/at_exit.h"
#include "crbase/functional/callback.h"
#include "crbase/functional/bind.h"
#include "crbase/message_loop/message_loop.h"
#include "crbase/message_loop/run_loop.h"
#include "crbase/threading/thread_task_runner_handle.h"
#include "crbase/threading/worker_pool/worker_pool.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/threading/platform_thread.h"
#include "crbase/time/time.h"
#include "crbase/timer/timer.h"
#include "crbase/buffer/pickle.h"
#include "crbase/win/msvc_import_libs.h"

using namespace std;

//------------------------------------------------------------------------------

namespace {

void InitializeLogging() {
  auto& settings = cr::logging::GetDefaultLoggingConfig();
  settings.logging_dest = cr::logging::kLogToStdErr;
  cr::logging::LoggingConfigInit(settings);
}

}  // namespace

//------------------------------------------------------------------------------

int main(int argc, char* argv) {
  cr::AtExitManager at_exit;
  cr::MessageLoopForIO message_loop;

  cr::Pickle pickle;
  pickle.WriteBool(false);
  pickle.WriteInt32(123456);
  pickle.WriteString("hello pickle");
  std::cout << "Pickle1 Data:" << (void*)(pickle.data()) << std::endl;
  std::cout << "Pickle1 Size:" << pickle.size() << std::endl;

  cr::Pickle pickle2 = pickle;
  pickle2.WriteBool(true);
  std::cout << "Pickle2 Data:" << (void*)(pickle2.data()) << std::endl;
  std::cout << "Pickle2 Size:" << pickle2.size() << std::endl;

  std::cout << cr::logging::SystemErrorCodeToString(ERROR_ACCESS_DENIED) << std::endl;
  std::cout << "Main Tid=" << cr::PlatformThread::CurrentId() << std::endl;

  cr::StringPiece16 a(L"ljlkjlkjklasdas");
  std::string b;
  cr::WideToUTF8(a.data(), a.length(), &b);
  std::cout << b << std::endl;

  cr::RepeatingTimer timer;

  timer.SetTaskRunner(message_loop.task_runner());
  timer.Start(
    CR_FROM("timer"), cr::TimeDelta::FromSeconds(2),
    cr::BindRepeating([] {std::cout << cr::Time::Now() << std::endl; }));

  cr::WorkerPool::PostTask(
    CR_FROM("11"), cr::BindOnce([] {
    std::cout << "worker pool!!" << cr::PlatformThread::CurrentId() << std::endl;
  }), true);

  cr::RunLoop().Run();
  return 0;
}

//------------------------------------------------------------------------------

//
// Outputs:
//
// create(false) returned empty
// create2(true) returned Godzilla
//
#include "crbase/at_exit.h"
#include "crbase/strings/stringprintf.h"
#include "crbase_runtime/run_loop.h"
#include "crbase_runtime/task/single_thread_task_executor.h"
#include "crbase_runtime/threading/thread_task_runner_handle.h"
#include "crbase_runtime/timer/timer.h"

int main(int argc, char* argv[]) {
  cr::AtExitManager at_exit_manager;

  cr::SingleThreadTaskExecutor task_executor(cr::MessagePumpType::IO);

  cr::RepeatingTimer timer;
  timer.SetTaskRunner(cr::ThreadTaskRunnerHandle::Get());
  timer.Start(
      CR_FROM_HERE, 
      cr::TimeDelta::FromSeconds(2), 
      cr::BindRepeating([]{
        static int times = 1;
        std::string msg = cr::StringPrintf("hello chromium, %d!!\n", times++);
        printf(msg.data(), times++);
      }));

  cr::ThreadTaskRunnerHandle::Get()->PostTask(
      CR_FROM_HERE,
      cr::BindOnce([]{
        printf("task from PostTask!\n");
      }));
  cr::RunLoop().Run(CR_FROM_HERE);
  return 0;
}
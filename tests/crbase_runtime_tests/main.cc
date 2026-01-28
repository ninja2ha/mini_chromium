#include "crbase/at_exit.h"
#include "crbase_runtime/run_loop.h"
#include "crbase_runtime/task/single_thread_task_executor.h"
#include "crbase_runtime/single_thread_task_runner.h"
#include "crbase_runtime/timer/timer.h"

int main(int argc, char* argv[]) {
  cr::AtExitManager at_exit_manager;

  cr::SingleThreadTaskExecutor task_executor(cr::MessagePumpType::UI);

  cr::RepeatingTimer timer;
  timer.SetTaskRunner(task_executor.task_runner());
  timer.Start(
      CR_FROM_HERE, 
      cr::TimeDelta::FromSeconds(2), 
      cr::BindRepeating([]{
        static int times = 1;
        printf("hello chromium, %d!!\n", times++);
      }));
  cr::RunLoop().Run(CR_FROM_HERE);
  return 0;
}
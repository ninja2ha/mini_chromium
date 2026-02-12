#include "crbase/at_exit.h"

#include "crbase_runtime/task/single_thread_task_executor.h"
#include "crbase_runtime/run_loop.h"

#include "cripc/named_platform_channel.h"
#include "cripc/connection_params.h"
#include "cripc/channel.h"

int main(int argc, char* argv[]) {
  cr::AtExitManager at_exit_manager;

  {
    cr::SingleThreadTaskExecutor task_executeor(cr::MessagePumpType::IO);

    mojo::NamedPlatformChannel::Options options;
    options.server_name = 
        mojo::NamedPlatformChannel::ServerNameFromUTF8("cripc");
    mojo::core::ConnectionParams connection_params(
        mojo::NamedPlatformChannel(options).TakeServerEndpoint());
    
    mojo::core::Channel::Create(nullptr, std::move(connection_params), 
                                task_executeor.task_runner());
    cr::RunLoop run_loop;
    run_loop.Run(CR_FROM_HERE);
  }
  return 0;
}
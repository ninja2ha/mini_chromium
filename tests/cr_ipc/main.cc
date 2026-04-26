#include "cr_base/logging/logging.h"
#include "cr_base/at_exit.h"

#include "cr_event/task/single_thread_task_executor.h"
#include "cr_event/run_loop.h"

#include "cr_ipc/named_channel.h"
#include "cr_ipc/connection_params.h"
#include "cr_ipc/channel.h"

namespace {

class IPCServerHandle : public cripc::Channel::Delegate {
 public:
  cripc::Channel::DispatchResult OnChannelMessage(
      const void* payload, size_t payload_size, size_t* size_hint) override {
    CR_LOG(Info) << "[IPC SERVER]: Got message, msg:" << (const char*)payload;
    *size_hint = payload_size;
    return cripc::Channel::DispatchResult::kOK;
  }

  void OnChannelError(cripc::Channel::Error error) override {
    CR_LOG(Warning) << "[IPC SERVER] Channel closed";
  }
};

class IPCClientHandle : public cripc::Channel::Delegate {
 public:
   cripc::Channel::DispatchResult OnChannelMessage(
      const void* payload, size_t payload_size, size_t* size_hint) override {
    CR_LOG(Info) << "[IPC CLIENT]: Got message, msg:" << (const char*)payload;
    *size_hint = payload_size;
    return cripc::Channel::DispatchResult::kOK;
   }

  void OnChannelError(cripc::Channel::Error error) override {
    CR_LOG(Warning) << "[IPC CLIENT] Channel closed";
  }
};

}  // namesapce

int main(int argc, char* argv[]) {
  CR_DEFAULT_LOGGING_CONFIG.logging_dest = cr::logging::LOG_TO_STDERR;

  cr::AtExitManager at_exit_manager;
  cr::SingleThreadTaskExecutor task_executeor(cr::MessagePumpType::IO);

  auto ipc_server_name = cripc::NamedChannel::ServerNameFromUTF8("cripc");

  // -- ipc server -------------------------------------------------------------
  cripc::NamedChannel::Options srv_options;
  srv_options.server_name = ipc_server_name;

  cripc::ConnectionParams srv_connection_params(
      cripc::NamedChannel(srv_options).TakeServerEndpoint());
    
  auto srv_handle = cr::WrapUnique(new IPCServerHandle);
  cr::RefPtr<cripc::Channel> srv_channel = 
      cripc::Channel::Create(srv_handle.get(), 
                             std::move(srv_connection_params),
                             task_executeor.task_runner());
  srv_channel->Write(
      cr::MakeRefCounted<cr::StringIOBuffer>(
          cr::Span<const char>("hello i am server.")));
  srv_channel->Start();

  // -- ipc client -------------------------------------------------------------
  cripc::ConnectionParams client_connection_params(
      cripc::NamedChannel::ConnectToServer(ipc_server_name));
  
  auto client_handle = cr::WrapUnique(new IPCClientHandle);
  cr::RefPtr<cripc::Channel> client_channel =
      cripc::Channel::Create(client_handle.get(),
                             std::move(client_connection_params),
                             task_executeor.task_runner());
  client_channel->Start();
  client_channel->Write(
      cr::MakeRefCounted<cr::StringIOBuffer>(
          cr::Span<const char>("hello i am client.")));

  // -- run loop ---------------------------------------------------------------
  cr::RunLoop run_loop;
  run_loop.Run(CR_FROM_HERE);

  return 0;
}
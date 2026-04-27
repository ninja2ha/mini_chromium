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
    CR_LOG(Info) << "[IPC SERVER]: Got message, msg:" 
                 << cr::StringPiece(reinterpret_cast<const char*>(payload), 
                                    payload_size);
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
    CR_LOG(Info) << "[IPC CLIENT]: Got message, msg:" 
                 << cr::StringPiece(reinterpret_cast<const char*>(payload), 
                                    payload_size);
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
  auto srv_msg_ptr = cr::WrapUnique(new cripc::Channel::Message);
  srv_msg_ptr->WriteString("hello i am server");
  srv_channel->Write(std::move(srv_msg_ptr));
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
  auto clt_msg_ptr = cr::WrapUnique(new cripc::Channel::Message);
  clt_msg_ptr->WriteString("hello i am client");
  client_channel->Write(std::move(clt_msg_ptr));

  // -- run loop ---------------------------------------------------------------
  cr::RunLoop run_loop;
  run_loop.Run(CR_FROM_HERE);

  return 0;
}
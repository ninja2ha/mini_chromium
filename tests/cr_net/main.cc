#include "cr_base/logging/logging.h"
#include "cr_base/at_exit.h"
#include "cr_base/memory/ptr_util.h"

#include "cr_event/run_loop.h"
#include "cr_event/single_thread_task_runner.h"
#include "cr_event/task/single_thread_task_executor.h"

#include "cr_net/tcp/tcp_connection.h"
#include "cr_net/tcp/tcp_server.h"
#include "cr_net/socket/tcp/tcp_server_socket.h"

#include "cr_net/tcp/tcp_client.h"
#include "cr_net/socket/tcp/tcp_client_socket.h"

////////////////////////////////////////////////////////////////////////////////

namespace {

class TCPServerHandle : public crnet::TCPServer::Delegate {
 public:
  // -- crnet::TCPServer::Delegate  --

  void OnAccept(crnet::TCPConnection* connection) override;
  int OnReceiveData(crnet::TCPConnection* connection, 
                    const char* data, 
                    int data_len) override;
  void OnClose(crnet::TCPConnection* connection) override;
};

//
void TCPServerHandle::OnAccept(crnet::TCPConnection* connection) {
  CR_LOG(Info) << "[SERVER] Got connection, id:" << connection->id();
}

//
int TCPServerHandle::OnReceiveData(crnet::TCPConnection* connection, 
                                   const char* data, 
                                   int data_len) {
  return data_len; // readed bytes!
}

//
void TCPServerHandle::OnClose(crnet::TCPConnection* connection) {
  CR_LOG(Info) << "[SERVER] Connection was closing, id:" << connection->id();
}

// --

class TCPClientHandle : public crnet::TCPClient::Delegate {
 public:
  void OnConnect(int rv) override;
  int OnReceiveData(const char* data, int data_len) override;
  void OnClose() override;
};


void TCPClientHandle::OnConnect(int rv) {
  CR_LOG(Info) << "[CLIENT] Result of connection: " << crnet::ErrorToString(rv);
}

int TCPClientHandle::OnReceiveData(const char* data, int data_len) {
  return data_len; // readed bytes.
}

void TCPClientHandle::OnClose() {
  CR_LOG(Info) << "[CLIENT] Disconnect with server.";
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
  CR_DEFAULT_LOGGING_CONFIG.logging_dest = cr::logging::LOG_TO_STDERR;

  cr::AtExitManager at_exit;
  cr::SingleThreadTaskExecutor task_executor(cr::MessagePumpType::IO);

  int net_error = 0;

  // - server
  auto server_handle = cr::WrapUnique(new TCPServerHandle());
  auto socket = cr::WrapUnique(new crnet::TCPServerSocket());

  net_error = socket->ListenWithAddressAndPort("::1", 3838, SOMAXCONN);
  CR_LOG(Info) << "[SERVER] Listen with address and port, Result: " 
               << crnet::ErrorToString(net_error);
  
  crnet::IPEndPoint bind_addr;
  if (socket->GetLocalAddress(&bind_addr) == crnet::OK) {
    CR_LOG(Info) << "[SERVER] Listening on the address: " 
                 << bind_addr.ToString();
  }

  crnet::TCPServer server(std::move(socket), server_handle.get());

  // - client
  auto client_handle = cr::WrapUnique(new TCPClientHandle());

  auto client_socket = cr::WrapUnique(new crnet::TCPClientSocket(
      crnet::AddressList::CreateFromIPLiteral("::1", 3838), nullptr));
  crnet::TCPClient client(std::move(client_socket), client_handle.get());

  cr::RunLoop run_loop;
  run_loop.Run(CR_FROM_HERE);
  return 0;
}


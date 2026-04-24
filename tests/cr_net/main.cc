#include "cr_base/logging/logging.h"
#include "cr_base/at_exit.h"
#include "cr_base/memory/ptr_util.h"
#include "cr_base/containers/span.h"

#include "cr_event/run_loop.h"
#include "cr_event/single_thread_task_runner.h"
#include "cr_event/task/single_thread_task_executor.h"

#include "cr_net/tcp/tcp_connection.h"
#include "cr_net/tcp/tcp_server.h"
#include "cr_net/socket/tcp/tcp_server_socket.h"

#include "cr_net/udp/udp_server.h"
#include "cr_net/udp/udp_client.h"
#include "cr_net/socket/udp/udp_server_socket.h"
#include "cr_net/socket/udp/udp_client_socket.h"

#include "cr_net/tcp/tcp_client.h"
#include "cr_net/socket/tcp/tcp_client_socket.h"

////////////////////////////////////////////////////////////////////////////////

namespace {

// --- TCP Server --------------------------------------------------------------

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
  CR_LOG(Info) << "[TCP SERVER] Got connection, id:" 
               << connection->id();
}

//
int TCPServerHandle::OnReceiveData(crnet::TCPConnection* connection, 
                                   const char* data, 
                                   int data_len) {
  return data_len; // readed bytes!
}

//
void TCPServerHandle::OnClose(crnet::TCPConnection* connection) {
  CR_LOG(Info) << "[TCP SERVER] Connection was closing, id:" 
               << connection->id();
}

// --- TCP Client --------------------------------------------------------------

class TCPClientHandle : public crnet::TCPClient::Delegate {
 public:
  void OnConnect(int rv) override;
  int OnReceiveData(const char* data, int data_len) override;
  void OnClose() override;
};


void TCPClientHandle::OnConnect(int rv) {
  CR_LOG(Info) << "[TCP CLIENT] Result of connection: " 
               << crnet::ErrorToString(rv);
}

int TCPClientHandle::OnReceiveData(const char* data, int data_len) {
  return data_len; // readed bytes.
}

void TCPClientHandle::OnClose() {
  CR_LOG(Info) << "[TCP CLIENT] Disconnect with server.";
}

// --- UDP Server --------------------------------------------------------------

class UDPServerHandle : public crnet::UDPServer::Delegate {
 public:
  void OnReceiveData(const crnet::IPEndPoint& end_point,
                     const char* data,
                     int data_len) override;

  void SetServer(crnet::UDPServer* server) {
    server_ = server;
  }

 private:
  crnet::UDPServer* server_ = nullptr;
};

void UDPServerHandle::OnReceiveData(const crnet::IPEndPoint& end_point,
                                    const char* data,
                                    int data_len) {
  CR_LOG(Info) << "[UDP SERVER]: Got message from[" << end_point.ToString()
               << "], msg=>" << cr::StringPiece(data, data_len);
  if (server_) {
    std::string msg(data, data_len);
    msg.append(", got it.");
    server_->SendData(end_point, cr::Span<const char>(msg));
  }
}

// --- UDP Client --------------------------------------------------------------

class UDPClientHandle : public crnet::UDPClient::Delegate {
 public:
  void OnReceiveData(const char* data,
                     int data_len) override;
};

void UDPClientHandle::OnReceiveData(const char* data,
                                    int data_len) {
  CR_LOG(Info) << "[UDP CLIENT]: Got message=>" 
               << cr::StringPiece(data, data_len);
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
  CR_DEFAULT_LOGGING_CONFIG.logging_dest = cr::logging::LOG_TO_STDERR;

  cr::AtExitManager at_exit;
  cr::SingleThreadTaskExecutor task_executor(cr::MessagePumpType::IO);

  int net_error = 0;

  // --- tcp server ------------------------------------------------------------

  auto tcp_server_handle = cr::WrapUnique(new TCPServerHandle());
  auto tcp_server_socket = cr::WrapUnique(new crnet::TCPServerSocket());

  net_error = 
      tcp_server_socket->ListenWithAddressAndPort("::1", 3838, SOMAXCONN);
  CR_LOG(Info) << "[TCP SERVER] Listen with address and port, Result: " 
               << crnet::ErrorToString(net_error);
  
  crnet::IPEndPoint tcp_bind_addr;
  if (tcp_server_socket->GetLocalAddress(&tcp_bind_addr) == crnet::OK) {
    CR_LOG(Info) << "[TCP SERVER] Listening on the address: " 
                 << tcp_bind_addr.ToString();
  }

  crnet::TCPServer tcp_server(std::move(tcp_server_socket), 
                              tcp_server_handle.get());

  // --- tcp client ------------------------------------------------------------

  auto tcp_client_handle = cr::WrapUnique(new TCPClientHandle());

  auto tcp_client_socket = cr::WrapUnique(new crnet::TCPClientSocket(
      crnet::AddressList::CreateFromIPLiteral("::1", 3838), nullptr));
  crnet::TCPClient tcp_client(std::move(tcp_client_socket), 
                              tcp_client_handle.get());

  // -- udp server -------------------------------------------------------------

  auto udp_server_handle = cr::WrapUnique(new UDPServerHandle);
  auto udp_server_socket = cr::WrapUnique(new crnet::UDPServerSocket());
  net_error = udp_server_socket->ListenWithAddressAndPort("127.0.0.1", 3000);
  CR_LOG(Info) << "[UDP SERVER] Listen with address and port, Result: " 
               << crnet::ErrorToString(net_error);
  
  crnet::IPEndPoint udp_bind_addr;
  if (udp_server_socket->GetLocalAddress(&udp_bind_addr) == crnet::OK) {
    CR_LOG(Info) << "[UDP SERVER] Listening on the address: " 
                 << udp_bind_addr.ToString();
  }

  crnet::UDPServer udp_server(std::move(udp_server_socket), 
                              udp_server_handle.get());
  udp_server_handle->SetServer(&udp_server);

  // -- udp client -------------------------------------------------------------

  auto udp_client_handle = cr::WrapUnique(new UDPClientHandle);
  auto udp_client_socket = cr::WrapUnique(
      new crnet::UDPClientSocket(crnet::UDPClientSocket::RANDOM_BIND));
  net_error = udp_client_socket->ConnectWithAddressAndPort("127.0.0.1", 3000);
  crnet::UDPClient udp_client(std::move(udp_client_socket),
                              udp_client_handle.get());
  udp_client.SendData(cr::Span<const char>("hello udp server!"));

  // -- run loop ---------------------------------------------------------------

  cr::RunLoop run_loop;
  run_loop.Run(CR_FROM_HERE);
  return 0;
}


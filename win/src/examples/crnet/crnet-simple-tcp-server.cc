// Copyright (c) 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <memory>

#include "crbase/logging.h"
#include "crbase/at_exit.h"
#include "crbase/synchronization/lock.h"
#include "crbase/win/msvc_import_libs.h"

#include "crnet/socket/tcp/tcp_server_socket.h"
#include "crnet/tcp/tcp_connection.h"
#include "crnet/tcp/tcp_server.h"

#include "examples/common/logging_initializtion.h"
#include "examples/common/thread_helper.h"

////////////////////////////////////////////////////////////////////////////////

namespace example {

class SimpleTCPServer : public crnet::TCPServer::Delegate {
 public:
  SimpleTCPServer();
  virtual ~SimpleTCPServer();

  // called on ui | io thread.
  void Start(const std::string& ipaddr, uint16_t port);
  void SendMsg(const std::string& msg);

 private:
  // called on ui thread.
  void HandleStartResult(int rv);

 protected:
  // TCPServer::Delegate implements.
  // called on io thread.
  void OnAccept(crnet::TCPConnection* connection) override;
  int OnTranslateData(crnet::TCPConnection* connection, 
                      const char* data, int data_len) override;
  void OnClose(crnet::TCPConnection* connection) override;

 private:
  // called on io thread.
  void DestoryIOThreadObject();


  // io thread members:
  cr::Lock lock_;
  crnet::IPEndPoint bind_addr_;     // guard by |lock_|.
  crnet::TCPConnection::ID id_ = 0; // guard by |lock_|.
  std::unique_ptr<crnet::TCPServer> tcp_server_;
  IOThreadHelper io_thread_; // destorying first
};

SimpleTCPServer::SimpleTCPServer() : io_thread_("io-thread") {
  CR_CHECK(io_thread_.Start(true));
}

SimpleTCPServer::~SimpleTCPServer() = default;

// -- ui & io thread --
void SimpleTCPServer::Start(const std::string& ipaddr, uint16_t port) {
  if (MainThreadHelper::Get()->IsOnCurrentlyThread()) {
    io_thread_.task_runner()->PostTask(
        CR_FROM_HERE,
        cr::BindOnce(
            &SimpleTCPServer::Start, cr::Unretained(this), ipaddr, port));
    return;
  }

  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  std::unique_ptr<crnet::TransportServerSocket> server_socket = 
      std::make_unique<crnet::TCPServerSocket>();
  int rv = server_socket->ListenWithAddressAndPort(ipaddr, port, true);
  do {
    if (rv != crnet::OK) 
      break;

    crnet::IPEndPoint bind_addr;
    rv = server_socket->GetLocalAddress(&bind_addr);
    if (rv != crnet::OK)
      break;

    {
      cr::AutoLock autolock(lock_);
      bind_addr_ = std::move(bind_addr);
    }

    // starting accept tcp connections.
    tcp_server_ = 
        std::make_unique<crnet::TCPServer>(std::move(server_socket), this);
  } while (0);

  // notify result.
  MainThreadHelper::Get()->task_runner()->PostTask(
      CR_FROM_HERE,
      cr::BindOnce(
          &SimpleTCPServer::HandleStartResult, cr::Unretained(this), rv));
}

// -- ui & io thread --
void SimpleTCPServer::SendMsg(const std::string& msg) {
  if (MainThreadHelper::Get()->IsOnCurrentlyThread()) {
    io_thread_.task_runner()->PostTask(
        CR_FROM_HERE,
        cr::BindOnce(
            &SimpleTCPServer::SendMsg, cr::Unretained(this), msg));
    return;
  }

  CR_DCHECK(io_thread_.IsOnCurrentlyThread());
  tcp_server_->SendDataToAll(msg);
}

// -- ui thread --
void SimpleTCPServer::HandleStartResult(int rv) {
  CR_DCHECK(MainThreadHelper::Get()->IsOnCurrentlyThread());

  if (rv != crnet::OK) {
    MainThreadHelper::Get()->AsyncQuit();
    return;
  }

  crnet::IPEndPoint bind_addr;
  {
    cr::AutoLock autolock(lock_);
    bind_addr = bind_addr_;
  }

  CR_LOG(Info) << "Successed to start server. listening on " 
               << bind_addr.ToString();

  std::string mesage;
  for (;;) {
    if (!std::getline(std::cin, mesage))
      break;

    // sending message
    SendMsg(mesage);
  }

  io_thread_.task_runner()->PostTask(
      CR_FROM_HERE,
      cr::BindOnce(
          &SimpleTCPServer::DestoryIOThreadObject, cr::Unretained(this)));
  MainThreadHelper::Get()->AsyncQuit();
}

// -- io thread --
void SimpleTCPServer::OnAccept(crnet::TCPConnection* connection) {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  {
    cr::AutoLock autolock(lock_);
    id_ = connection->id();
  }
  CR_LOG(Info) << "got new connection, id=" << connection->id();
}

// -- io thread --
int SimpleTCPServer::OnTranslateData(
    crnet::TCPConnection* connection,
    const char* data, 
    int data_len) { 
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  std::string message;
  message.assign(data, data_len);

  CR_LOG(Info) << "message[" << connection->id() << "]:" << message;
  return data_len;
}

// -- io thread --
void SimpleTCPServer::OnClose(crnet::TCPConnection* connection) {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());
  
  CR_LOG(Info) << "connection closed, id=" << connection->id();
}

// -- io thread --
void SimpleTCPServer::DestoryIOThreadObject() {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  {
    cr::AutoLock autolock(lock_);
    std::move(bind_addr_);
  }

  tcp_server_.reset();
}

// runner
int RunSimpleTCPServer() {
  MainThreadHelper main_thread(cr::MessageLoop::TYPE_UI, true);

  SimpleTCPServer tcp_server;
  tcp_server.Start("::1", 3838);

  main_thread.Run();
  return 0;
}

}  // namespace example

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
  example::InitLogging();

  cr::AtExitManager at_exit;
  return example::RunSimpleTCPServer();
}
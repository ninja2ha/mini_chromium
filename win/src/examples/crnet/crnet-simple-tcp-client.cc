// Copyright (c) 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <memory>

#include "crbase/logging.h"
#include "crbase/at_exit.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/atomic/atomic_flag.h"
#include "crbase/win/msvc_import_libs.h"
#include "crbase/build_platform.h"

#include "crnet/base/ip_address.h"
#include "crnet/base/address_list.h"
#include "crnet/socket/client_socket_factory.h"
#include "crnet/tcp/tcp_connection.h"
#include "crnet/tcp/tcp_client.h"

#include "examples/common/logging_initializtion.h"
#include "examples/common/thread_helper.h"

////////////////////////////////////////////////////////////////////////////////

namespace example {

class SimpleTCPClient : public crnet::TCPClient::Delegate {
 public:
  SimpleTCPClient();
  virtual ~SimpleTCPClient();

  // called on ui | io thread.
  void Connect(const std::string& ipaddr, uint16_t port);
  void SendMsg(const std::string& msg);

 private:
  // called on ui thread.
  void HandleConnectResult(int rv);

 protected:
  // TCPClient::Delegate implements.
  // called on io thread.
  void OnConnect(int rv) override;
  int OnTranslateData(const char* data, int data_len) override;
  void OnClose() override;

 private:
  // called on io thread.
  void DestoryIOThreadObject();

  cr::AtomicFlag closed_flag_;
  // io thread members:
  std::unique_ptr<crnet::TCPClient> tcp_client_;
  IOThreadHelper io_thread_; // destorying first
};

SimpleTCPClient::SimpleTCPClient() : io_thread_("io-thread") {
  CR_CHECK(io_thread_.Start(true));
}

SimpleTCPClient::~SimpleTCPClient() = default;

// -- ui & io thread --
void SimpleTCPClient::Connect(const std::string& ipaddr, uint16_t port) {
  if (MainThreadHelper::Get()->IsOnCurrentlyThread()) {
    io_thread_.task_runner()->PostTask(
        CR_FROM_HERE,
        cr::BindOnce(
            &SimpleTCPClient::Connect, cr::Unretained(this), ipaddr, port));
    return;
  }

  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  crnet::AddressList address_list = 
      crnet::AddressList::CreateFromIPLiteral(ipaddr, port);

  std::unique_ptr<crnet::TransportClientSocket> client_socket = 
      crnet::ClientSocketFactory::GetDefaultFactory()
          ->CreateTransportClientSocket(address_list, nullptr);
  
  tcp_client_ = cr::WrapUnique(
      new crnet::TCPClient(std::move(client_socket), this));
}

// -- ui & io thread --
void SimpleTCPClient::SendMsg(const std::string& msg) {
  if (MainThreadHelper::Get()->IsOnCurrentlyThread()) {
    io_thread_.task_runner()->PostTask(
        CR_FROM_HERE,
        cr::BindOnce(
            &SimpleTCPClient::SendMsg, cr::Unretained(this), msg));
    return;
  }

  CR_DCHECK(io_thread_.IsOnCurrentlyThread());
  tcp_client_->SendData(msg);
}

// -- ui thread --
void SimpleTCPClient::HandleConnectResult(int rv) {
  CR_DCHECK(MainThreadHelper::Get()->IsOnCurrentlyThread());

  cr::OnceClosure destory_cb = cr::BindOnce(
      &SimpleTCPClient::DestoryIOThreadObject, cr::Unretained(this));

  if (rv != crnet::OK) {
    io_thread_.task_runner()->PostTask(CR_FROM_HERE, std::move(destory_cb));
    MainThreadHelper::Get()->AsyncQuit();
    return;
  }

  CR_LOG(Info) << "established with server. sending message now.";

  std::string mesage;
  for (;;) {
    if (!std::getline(std::cin, mesage))
      break;

    if (closed_flag_.IsSet())
      break;

    // sending message
    SendMsg(mesage);
  }

  io_thread_.task_runner()->PostTask(CR_FROM_HERE, std::move(destory_cb));
  MainThreadHelper::Get()->AsyncQuit();
}

// -- io thread --
void SimpleTCPClient::OnConnect(int rv) {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  MainThreadHelper::Get()->task_runner()->PostTask(
      CR_FROM_HERE, 
      cr::BindOnce(&SimpleTCPClient::HandleConnectResult, cr::Unretained(this), 
                   rv));
}

// -- io thread --
int SimpleTCPClient::OnTranslateData(const char* data, int data_len) { 
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  std::string message;
  message.assign(data, data_len);

  CR_LOG(Info) << "message:" << message;
  return data_len;
}

// -- io thread --
void SimpleTCPClient::OnClose() {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());
  closed_flag_.Set();
  CR_LOG(Info) << "disconnected with server.";
}

// -- io thread --
void SimpleTCPClient::DestoryIOThreadObject() {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  tcp_client_.reset();
}

// runner
int RunSimpleTCPServer() {
  MainThreadHelper main_thread(cr::MessageLoop::TYPE_UI, true);

  SimpleTCPClient tcp_server;
  tcp_server.Connect("::1", 3838);

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
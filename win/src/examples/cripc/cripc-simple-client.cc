// Copyright (c) 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <iostream>
#include <memory>

#include "crbase/logging.h"
#include "crbase/at_exit.h"
#include "crbase/functional/bind.h"
#include "crbase/atomic/atomic_flag.h"
#include "crbase/buffer/io_buffer.h"
#include "crbase/win/msvc_import_libs.h"

#include "cripc/ipc_channel_handle.h"
#include "cripc/ipc_channel.h"

#include "examples/common/logging_initializtion.h" 
#include "examples/common/thread_helper.h" 

////////////////////////////////////////////////////////////////////////////////

namespace example {

class SimpleIPCClient final : public cripc::Channel::Delegate {
 public:
  SimpleIPCClient();
  virtual ~SimpleIPCClient();
  
  // called on main thread.
  void Connect(const std::string& channel_id);
  void SendMsg(const std::string& message);

 private:
  // called on main thread.
  void HandleConnectResult(bool connected);

 protected:
  // Channel::Delegate implements 
  // called on io thread.

  // Called when input data is received.  Returns the numbers of bytes 
  // handled. The value is less-than 0 while error occured. 
  int OnChannelTranslateData(const char* data, int data_len) override;

  // Called when an error is detected that causes the channel to close.
  void OnChannelError();
  
  void DestoryChannel();

 private:
  cr::AtomicFlag channel_err_;
  std::unique_ptr<cripc::Channel> channel_;
  IOThreadHelper io_thread_; // destorying first
};

SimpleIPCClient::SimpleIPCClient() : io_thread_("ipc-thread") {
  CR_CHECK(io_thread_.Start(true));
}

SimpleIPCClient::~SimpleIPCClient() = default;

void SimpleIPCClient::Connect(const std::string& channel_id) {
  // currently on the main thread
  if (MainThreadHelper::Get()->IsOnCurrentlyThread()) {
    // connecting in the io thread
    io_thread_.task_runner()->PostTask(
        CR_FROM_HERE, 
        cr::BindOnce(&SimpleIPCClient::Connect, cr::Unretained(this), 
                     channel_id));
    return;
  }

  // checks whether on the io thread.
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  cripc::ChannelHandle handle(channel_id);
  channel_ = cripc::Channel::CreateNamedClient(handle, this);
  bool connected = channel_->Connect();
  if (!connected) channel_.reset();

  // runing 'HandleConnectResult' in the main thread.
  MainThreadHelper::Get()->task_runner()->PostTask(
      CR_FROM_HERE,
      cr::BindOnce(&SimpleIPCClient::HandleConnectResult, cr::Unretained(this),
                    connected));
}

void SimpleIPCClient::SendMsg(const std::string& message) {
  // do it in the io thread.
  if (MainThreadHelper::Get()->IsOnCurrentlyThread()) {
    io_thread_.task_runner()->PostTask(
        CR_FROM_HERE, 
        cr::BindOnce(&SimpleIPCClient::SendMsg, cr::Unretained(this),
                     message));
    return;
  }

  // checks whether on the io thread.
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());
  if (!channel_)
    return; // the channel has been closed!

  std::unique_ptr<cr::Pickle> pickle(new cr::Pickle);
  pickle->WriteString(message);
  channel_->Send(
      cr::MakeRefCounted<cr::PickledIOBuffer>(std::move(pickle)));
}

void SimpleIPCClient::HandleConnectResult(bool connected) {
  CR_DCHECK(MainThreadHelper::Get()->IsOnCurrentlyThread());
  if (!connected) {
    // connection failed. exit the application. 
    MainThreadHelper::Get()->AsyncQuit();
    return;
  }

  CR_LOG(Info) << "Successed to connect ipc server. waiting for message.";

  // connection successed. let`s communicate with ipc server. 
  std::string message;
  for (;;) {
    std::cout << "[input]:";
    if (!std::getline(std::cin, message))
      break;

    if (channel_err_.IsSet())
      break;

    SendMsg(message);
  }

  // destorying |channel_| in the io thread.
  io_thread_.task_runner()->PostTask(
      CR_FROM_HERE, 
      cr::BindOnce(&SimpleIPCClient::DestoryChannel, cr::Unretained(this)));
  MainThreadHelper::Get()->AsyncQuit();
}

// overrides
int SimpleIPCClient::OnChannelTranslateData(const char* data, int data_len) {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());

  const cr::Pickle pickle(data, static_cast<size_t>(data_len));
  cr::PickleIterator reader(pickle);

  std::string message;
  if (reader.ReadString(&message)) {
    CR_LOG(Info) << "Server=> message:" << message;
    return static_cast<int>(pickle.size());
  }

  // throwing simplely a error while not all data are received.
  return -1;
}

void SimpleIPCClient::OnChannelError() {
  // std::getline blocked main thread message. so just set a flag to quit
  // the application after std::getline called.
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());
  CR_LOG(Info) << "channel closed!";

  io_thread_.task_runner()->DeleteSoon(CR_FROM_HERE, std::move(channel_));
  channel_err_.Set();
}

void SimpleIPCClient::DestoryChannel() {
  CR_DCHECK(io_thread_.IsOnCurrentlyThread());
  channel_.reset();
}

int RunSimpleIPCClient() {
  MainThreadHelper main_thread(cr::MessageLoop::TYPE_UI, true);

  SimpleIPCClient ipc_client;
  ipc_client.Connect("simple-ipc");

  main_thread.Run();
  return 0;
}

}  // namespace example

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
  example::InitLogging();

  cr::AtExitManager at_exit;
  return example::RunSimpleIPCClient();
}

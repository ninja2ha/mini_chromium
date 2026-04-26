// Copyright 2004 The WebRTC Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef MINI_CHROMIUM_SRC_CRNET_UDP_UDP_SERVER_H_
#define MINI_CHROMIUM_SRC_CRNET_UDP_UDP_SERVER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "cr_base/memory/weak_ptr.h"
#include "cr_base/containers/span.h"

#include "cr_event/io_buffer.h"

#include "cr_net/net_export.h"
#include "cr_net/base/ip_endpoint.h"
#include "cr_net/socket/udp/datagram_server_socket.h"

namespace crnet {

class CRNET_EXPORT UDPServer {
 public:
  struct Delegate {
    virtual ~Delegate() {};

    virtual void OnReceiveData(const IPEndPoint& endpoint,
                               const char* data, 
                               int data_len) = 0;
  };

  UDPServer(const UDPServer&) = delete;
  UDPServer& operator=(const UDPServer&) = delete;

  UDPServer(std::unique_ptr<DatagramServerSocket> socket,
            Delegate* delegate);
  ~UDPServer();

  void SendData(const IPEndPoint& endpoint, cr::Span<const char> data);

  // set the max receive buffer size for per-recv-from data.
  // the default size are 1024 bytes.
  // call it behand the Constructor.
  void SetReceiveBufferSize(int32_t size);

  // set the max send data size. the default value are 1024 * 1024 bytes
  void SetSendBufferSize(int32_t size);

 private:
   // -- read --
  void DoRecvFromLoop();
  void OnRecvFromCompleted(int rv);
  int HandleRecvFromResult(int rv);

  // -- write --
  void DoWriteLoop();
  void OnWriteCompleted(int rv);
  int HandleWriteResult(int rv);

  // -- handle --
  void HandleReadedData(const IPEndPoint& endpoint, 
                        const char* data, 
                        int size);

 private:
  Delegate* delegate_;

  IPEndPoint endpoint_;
  std::unique_ptr<DatagramServerSocket> socket_;

  // -- write --
  cr::RefPtr<cr::QueuedWriteIOBuffer> write_queue_;
  cr::Queue<IPEndPoint> write_queue_ep_;

  // -- read --
  cr::RefPtr<cr::ReadIOBuffer> read_buf_;

  cr::WeakPtrFactory<UDPServer> weak_ptr_factory_{ this };
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_UDP_UDP_SERVER_H_
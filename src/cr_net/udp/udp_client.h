// Copyright 2004 The WebRTC Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef MINI_CHROMIUM_SRC_CRNET_UDP_UDP_CLIENT_H_
#define MINI_CHROMIUM_SRC_CRNET_UDP_UDP_CLIENT_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "cr_base/memory/weak_ptr.h"
#include "cr_base/containers/span.h"

#include "cr_event/io_buffer.h"

#include "cr_net/net_export.h"
#include "cr_net/base/ip_endpoint.h"
#include "cr_net/socket/udp/datagram_client_socket.h"

namespace crnet {

class CRNET_EXPORT UDPClient {
 public:
  struct Delegate {
    virtual ~Delegate() {};
    virtual void OnReceiveData(const char* data, int data_len) = 0;
  };

  UDPClient(const UDPClient&) = delete;
  UDPClient& operator=(const UDPClient&) = delete;

  UDPClient(std::unique_ptr<DatagramClientSocket> socket,
            Delegate* delegate);
  ~UDPClient();

  void SendData(cr::Span<const char> data);

  // set the max receive buffer size for read data.
  // the default size are 1024 bytes. call it behand the Constructor.
  void SetReceiveBufferSize(int32_t size);

  void SetSendBufferSize(int32_t size);

 private:
   // -- read --
  void DoReadLoop();
  void OnReadCompleted(int rv);
  int HandleReadResult(int rv);

  // -- write --
  void DoWriteLoop();
  void OnWriteCompleted(int rv);
  int HandleWriteResult(int rv);

  // -- handle --
  void HandleReadedData(const char* data, int size);

 private:
  Delegate* delegate_;

  std::unique_ptr<DatagramClientSocket> socket_;

  // -- write --
  cr::RefPtr<cr::QueuedWriteIOBuffer> write_queue_;

  // -- read --
  cr::RefPtr<cr::ReadIOBuffer> read_buf_;

  cr::WeakPtrFactory<UDPClient> weak_ptr_factory_{ this };
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_UDP_UDP_CLIENT_H_
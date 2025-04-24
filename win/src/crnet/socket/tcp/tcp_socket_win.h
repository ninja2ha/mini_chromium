// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SOCKET_WIN_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SOCKET_WIN_H_

#include <stdint.h>
#include <winsock2.h>

#include <memory>

#include "crbase/compiler_specific.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/threading/thread_checker.h"
#include "crbase/win/object_watcher.h"
#include "crnet/base/address_family.h"
#include "crnet/base/completion_once_callback.h"
#include "crnet/base/net_export.h"
#include "crnet/socket/socket_descriptor.h"
#include "crnet/socket/socket_performance_watcher.h"

namespace cr {
class IOBuffer;
}  // namespace cr

namespace crnet {

class AddressList;
class IPEndPoint;
///class SocketTag;

class CRNET_EXPORT TCPSocketWin : public cr::win::ObjectWatcher::Delegate {
 public:
  TCPSocketWin(const TCPSocketWin&) = delete;
  TCPSocketWin& operator=(const TCPSocketWin&) = delete;

  TCPSocketWin(
      std::unique_ptr<SocketPerformanceWatcher> socket_performance_watcher);
  ~TCPSocketWin() override;

  int Open(AddressFamily family);

  // Takes ownership of |socket|, which is known to already be connected to the
  // given peer address. However, peer address may be the empty address, for
  // compatibility. The given peer address will be returned by GetPeerAddress.
  int AdoptConnectedSocket(SocketDescriptor socket,
                           const IPEndPoint& peer_address);
  // Takes ownership of |socket|, which may or may not be open, bound, or
  // listening. The caller must determine the state of the socket based on its
  // provenance and act accordingly. The socket may have connections waiting
  // to be accepted, but must not be actually connected.
  int AdoptUnconnectedSocket(SocketDescriptor socket);

  int Bind(const IPEndPoint& address);

  int Listen(int backlog);
  int Accept(std::unique_ptr<TCPSocketWin>* socket,
             IPEndPoint* address,
             CompletionOnceCallback callback);

  int Connect(const IPEndPoint& address, CompletionOnceCallback callback);
  bool IsConnected() const;
  bool IsConnectedAndIdle() const;

  // Multiple outstanding requests are not supported.
  // Full duplex mode (reading and writing at the same time) is supported.
  int Read(cr::IOBuffer* buf, int buf_len, CompletionOnceCallback callback);
  int ReadIfReady(cr::IOBuffer* buf, 
                  int buf_len, 
                  CompletionOnceCallback callback);
  int CancelReadIfReady();
  int Write(cr::IOBuffer* buf,
            int buf_len,
            CompletionOnceCallback callback);

  int GetLocalAddress(IPEndPoint* address) const;
  int GetPeerAddress(IPEndPoint* address) const;

  // Sets various socket options.
  // The commonly used options for server listening sockets:
  // - SetExclusiveAddrUse().
  int SetDefaultOptionsForServer();
  // The commonly used options for client sockets and accepted sockets:
  // - SetNoDelay(true);
  // - SetKeepAlive(true, 45).
  void SetDefaultOptionsForClient();
  int SetExclusiveAddrUse();
  int SetReceiveBufferSize(int32_t size);
  int SetSendBufferSize(int32_t size);
  bool SetKeepAlive(bool enable, int delay);
  bool SetNoDelay(bool no_delay);

  // Gets the estimated RTT. Returns false if the RTT is
  // unavailable. May also return false when estimated RTT is 0.
  bool GetEstimatedRoundTripTime(cr::TimeDelta* out_rtt) const
      CR_WARN_UNUSED_RESULT;

  void Close();

  bool IsValid() const { return socket_ != INVALID_SOCKET; }

  // Detachs from the current thread, to allow the socket to be transferred to
  // a new thread. Should only be called when the object is no longer used by
  // the old thread.
  void DetachFromThread();

  // May return nullptr.
  SocketPerformanceWatcher* socket_performance_watcher() const {
    return socket_performance_watcher_.get();
  }

 private:
  class Core;

  // base::ObjectWatcher::Delegate implementation.
  void OnObjectSignaled(HANDLE object) override;

  int AcceptInternal(std::unique_ptr<TCPSocketWin>* socket,
                     IPEndPoint* address);

  int DoConnect();
  void DoConnectComplete(int result);

  void RetryRead(int rv);
  void DidCompleteConnect();
  void DidCompleteWrite();
  void DidSignalRead();

  SOCKET socket_;

  // |socket_performance_watcher_| may be nullptr.
  std::unique_ptr<SocketPerformanceWatcher> socket_performance_watcher_;

  HANDLE accept_event_;
  cr::win::ObjectWatcher accept_watcher_;

  std::unique_ptr<TCPSocketWin>* accept_socket_;
  IPEndPoint* accept_address_;
  CompletionOnceCallback accept_callback_;

  // The various states that the socket could be in.
  bool waiting_connect_;
  bool waiting_read_;
  bool waiting_write_;

  // The core of the socket that can live longer than the socket itself. We pass
  // resources to the Windows async IO functions and we have to make sure that
  // they are not destroyed while the OS still references them.
  cr::RefPtr<Core> core_;

  // External callback; called when connect or read is complete.
  CompletionOnceCallback read_callback_;

  // Non-null if a ReadIfReady() is to be completed asynchronously. This is an
  // external callback if user used ReadIfReady() instead of Read(), but a
  // wrapped callback on top of RetryRead() if Read() is used.
  CompletionOnceCallback read_if_ready_callback_;

  // External callback; called when write is complete.
  CompletionOnceCallback write_callback_;

  std::unique_ptr<IPEndPoint> peer_address_;
  // The OS error that a connect attempt last completed with.
  int connect_os_error_;

  bool logging_multiple_connect_attempts_;

  CR_THREAD_CHECKER(thread_checker_);
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_TCP_SOCKET_WIN_H_
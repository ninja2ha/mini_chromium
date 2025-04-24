// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_STREAM_SOCKET_H_
#define MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_STREAM_SOCKET_H_

#include <stdint.h>

#include "crbase/functional/bind.h"
#include "crbase/build_platform.h"
#include "crnet/base/net_errors.h"
#include "crnet/base/net_export.h"
#include "crnet/socket/connection_attempts.h"
#include "crnet/socket/socket.h"

namespace crnet {

class IPEndPoint;

class CRNET_EXPORT StreamSocket : public Socket {
 public:
  using BeforeConnectCallback = cr::RepeatingCallback<int()>;

  // This is used in DumpMemoryStats() to track the estimate of memory usage of
  // a socket.
  struct CRNET_EXPORT_PRIVATE SocketMemoryStats {
   public:
    SocketMemoryStats(const SocketMemoryStats&) = delete; 
    SocketMemoryStats& operator=(const SocketMemoryStats&) = delete;

    SocketMemoryStats();
    ~SocketMemoryStats();
    // Estimated total memory usage of this socket in bytes.
    size_t total_size;
    // Size of all buffers used by this socket in bytes.
    size_t buffer_size;
    // Number of certs used by this socket.
    size_t cert_count;
    // Total size of certs used by this socket in bytes.
    size_t cert_size;
  };

  ~StreamSocket() override {}

  // Sets a callback to be invoked before establishing a connection. This allows
  // setting options, like receive and send buffer size, when they will take
  // effect. The callback should return crnet::OK on success, and an error on
  // failure. It must not return crnet::ERR_IO_PENDING.
  //
  // If multiple connection attempts are made, the callback will be invoked for
  // each one.
  virtual void SetBeforeConnectCallback(
      const BeforeConnectCallback& before_connect_callback);

  // Called to establish a connection.  Returns OK if the connection could be
  // established synchronously.  Otherwise, ERR_IO_PENDING is returned and the
  // given callback will run asynchronously when the connection is established
  // or when an error occurs.  The result is some other error code if the
  // connection could not be established.
  //
  // The socket's Read and Write methods may not be called until Connect
  // succeeds.
  //
  // It is valid to call Connect on an already connected socket, in which case
  // OK is simply returned.
  //
  // Connect may also be called again after a call to the Disconnect method.
  //
  virtual int Connect(CompletionOnceCallback callback) = 0;

  // Called to disconnect a socket.  Does nothing if the socket is already
  // disconnected.  After calling Disconnect it is possible to call Connect
  // again to establish a new connection.
  //
  // If IO (Connect, Read, or Write) is pending when the socket is
  // disconnected, the pending IO is cancelled, and the completion callback
  // will not be called.
  virtual void Disconnect() = 0;

  // Called to test if the connection is still alive.  Returns false if a
  // connection wasn't established or the connection is dead.  True is returned
  // if the connection was terminated, but there is unread data in the incoming
  // buffer.
  virtual bool IsConnected() const = 0;

  // Called to test if the connection is still alive and idle.  Returns false
  // if a connection wasn't established, the connection is dead, or there is
  // unread data in the incoming buffer.
  virtual bool IsConnectedAndIdle() const = 0;

  // Copies the peer address to |address| and returns a network error code.
  // ERR_SOCKET_NOT_CONNECTED will be returned if the socket is not connected.
  virtual int GetPeerAddress(IPEndPoint* address) const = 0;

  // Copies the local address to |address| and returns a network error code.
  // ERR_SOCKET_NOT_CONNECTED will be returned if the socket is not bound.
  virtual int GetLocalAddress(IPEndPoint* address) const = 0;

  // Returns true if the socket ever had any reads or writes.  StreamSockets
  // layered on top of transport sockets should return if their own Read() or
  // Write() methods had been called, not the underlying transport's.
  virtual bool WasEverUsed() const = 0;

  // Overwrites |out| with the connection attempts made in the process of
  // connecting this socket.
  virtual void GetConnectionAttempts(ConnectionAttempts* out) const = 0;

  // Clears the socket's list of connection attempts.
  virtual void ClearConnectionAttempts() = 0;

  // Adds |attempts| to the socket's list of connection attempts.
  virtual void AddConnectionAttempts(const ConnectionAttempts& attempts) = 0;

  // Returns the total number of number bytes read by the socket. This only
  // counts the payload bytes. Transport headers are not counted. Returns
  // 0 if the socket does not implement the function. The count is reset when
  // Disconnect() is called.
  virtual int64_t GetTotalReceivedBytes() const = 0;

  // Dumps memory allocation stats into |stats|. |stats| can be assumed as being
  // default initialized upon entry. Implementations should override fields in
  // |stats|. Default implementation does nothing.
  virtual void DumpMemoryStats(SocketMemoryStats* stats) const {}

#if defined(MINI_CHROMIUM_OS_POSIX)
  // Apply |tag| to this socket. If socket isn't yet connected, tag will be
  // applied when socket is later connected. If Connect() fails or socket
  // is closed, tag is cleared. If this socket is layered upon or wraps an
  // underlying socket, |tag| will be applied to the underlying socket in the
  // same manner as if ApplySocketTag() was called on the underlying socket.
  // The tag can be applied at any time, in other words active sockets can be
  // retagged with a different tag. Sockets wrapping multiplexed sockets
  // (e.g. sockets who proxy through a QUIC or Spdy stream) cannot be tagged as
  // the tag would inadvertently affect other streams; calling ApplySocketTag()
  // in this case will result in CHECK(false).
  virtual void ApplySocketTag(const SocketTag& tag) = 0;
#endif
};

}  // namespace net

#endif  // MINI_CHROMIUM_SRC_CRNET_SOCKET_TCP_STREAM_SOCKET_H_
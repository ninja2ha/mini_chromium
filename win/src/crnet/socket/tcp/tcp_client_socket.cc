// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/socket/tcp/tcp_client_socket.h"

#include <utility>

#include "crbase/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/callback_helpers.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/time/time.h"
#include "crbase/buffer/io_buffer.h"
#include "crnet/base/ip_endpoint.h"
#include "crnet/base/net_errors.h"
#include "crnet/socket/socket_performance_watcher.h"

#if defined(TCP_CLIENT_SOCKET_OBSERVES_SUSPEND)
///#include "base/power_monitor/power_monitor.h"
#endif

namespace crnet {

TCPClientSocket::TCPClientSocket(
    const AddressList& addresses,
    std::unique_ptr<SocketPerformanceWatcher> socket_performance_watcher)
    : TCPClientSocket(
          std::make_unique<TCPSocket>(std::move(socket_performance_watcher)),
          addresses,
          -1 /* current_address_index */,
          nullptr /* bind_address */) {}

TCPClientSocket::TCPClientSocket(std::unique_ptr<TCPSocket> connected_socket,
                                 const IPEndPoint& peer_address)
    : TCPClientSocket(std::move(connected_socket),
                      AddressList(peer_address),
                      0 /* current_address_index */,
                      nullptr /* bind_address */) {}

TCPClientSocket::~TCPClientSocket() {
  Disconnect();
#if defined(TCP_CLIENT_SOCKET_OBSERVES_SUSPEND)
 /// base::PowerMonitor::RemoveObserver(this);
#endif  // defined(TCP_CLIENT_SOCKET_OBSERVES_SUSPEND)
}

std::unique_ptr<TCPClientSocket> TCPClientSocket::CreateFromBoundSocket(
    std::unique_ptr<TCPSocket> bound_socket,
    const AddressList& addresses,
    const IPEndPoint& bound_address) {
  return cr::WrapUnique(new TCPClientSocket(
      std::move(bound_socket), addresses, -1 /* current_address_index */,
      std::make_unique<IPEndPoint>(bound_address)));
}

int TCPClientSocket::Bind(const IPEndPoint& address) {
  if (current_address_index_ >= 0 || bind_address_) {
    // Cannot bind the socket if we are already connected or connecting.
    CR_NOTREACHED();
    return ERR_UNEXPECTED;
  }

  int result = OK;
  if (!socket_->IsValid()) {
    result = OpenSocket(address.GetFamily());
    if (result != OK)
      return result;
  }

  result = socket_->Bind(address);
  if (result != OK)
    return result;

  bind_address_.reset(new IPEndPoint(address));
  return OK;
}

bool TCPClientSocket::SetKeepAlive(bool enable, int delay) {
  return socket_->SetKeepAlive(enable, delay);
}

bool TCPClientSocket::SetNoDelay(bool no_delay) {
  return socket_->SetNoDelay(no_delay);
}

void TCPClientSocket::SetBeforeConnectCallback(
    const BeforeConnectCallback& before_connect_callback) {
  CR_DCHECK(CONNECT_STATE_NONE == next_connect_state_);
  before_connect_callback_ = before_connect_callback;
}

int TCPClientSocket::Connect(CompletionOnceCallback callback) {
  CR_DCHECK(!callback.is_null());

  // If connecting or already connected, then just return OK.
  if (socket_->IsValid() && current_address_index_ >= 0)
    return OK;

  CR_DCHECK(!read_callback_);
  CR_DCHECK(!write_callback_);

  if (was_disconnected_on_suspend_) {
    Disconnect();
    was_disconnected_on_suspend_ = false;
  }

  // We will try to connect to each address in addresses_. Start with the
  // first one in the list.
  next_connect_state_ = CONNECT_STATE_CONNECT;
  current_address_index_ = 0;

  int rv = DoConnectLoop(OK);
  if (rv == ERR_IO_PENDING) {
    connect_callback_ = std::move(callback);
  }

  return rv;
}

TCPClientSocket::TCPClientSocket(std::unique_ptr<TCPSocket> socket,
                                 const AddressList& addresses,
                                 int current_address_index,
                                 std::unique_ptr<IPEndPoint> bind_address)
    : socket_(std::move(socket)),
      bind_address_(std::move(bind_address)),
      addresses_(addresses),
      current_address_index_(-1),
      next_connect_state_(CONNECT_STATE_NONE),
      previously_disconnected_(false),
      total_received_bytes_(0),
      was_ever_used_(false),
      was_disconnected_on_suspend_(false) {
  CR_DCHECK(socket_);
  if (socket_->IsValid())
    socket_->SetDefaultOptionsForClient();
#if defined(TCP_CLIENT_SOCKET_OBSERVES_SUSPEND)
  ///base::PowerMonitor::AddObserver(this);
#endif  // defined(TCP_CLIENT_SOCKET_OBSERVES_SUSPEND)
}

int TCPClientSocket::ReadCommon(cr::IOBuffer* buf,
                                int buf_len,
                                CompletionOnceCallback callback,
                                bool read_if_ready) {
  CR_DCHECK(!callback.is_null());
  CR_DCHECK(read_callback_.is_null());

  if (was_disconnected_on_suspend_)
    return ERR_NETWORK_IO_SUSPENDED;

  // |socket_| is owned by |this| and the callback won't be run once |socket_|
  // is gone/closed. Therefore, it is safe to use base::Unretained() here.
  CompletionOnceCallback complete_read_callback =
      cr::BindOnce(&TCPClientSocket::DidCompleteRead, cr::Unretained(this));
  int result =
      read_if_ready
          ? socket_->ReadIfReady(buf, buf_len,
                                 std::move(complete_read_callback))
          : socket_->Read(buf, buf_len, std::move(complete_read_callback));
  if (result == ERR_IO_PENDING) {
    read_callback_ = std::move(callback);
  } else if (result > 0) {
    was_ever_used_ = true;
    total_received_bytes_ += result;
  }

  return result;
}

int TCPClientSocket::DoConnectLoop(int result) {
  CR_DCHECK(next_connect_state_ != CONNECT_STATE_NONE);

  int rv = result;
  do {
    ConnectState state = next_connect_state_;
    next_connect_state_ = CONNECT_STATE_NONE;
    switch (state) {
      case CONNECT_STATE_CONNECT:
        CR_DCHECK(OK == rv);
        rv = DoConnect();
        break;
      case CONNECT_STATE_CONNECT_COMPLETE:
        rv = DoConnectComplete(rv);
        break;
      default:
        CR_NOTREACHED() << "bad state " << state;
        rv = ERR_UNEXPECTED;
        break;
    }
  } while (rv != ERR_IO_PENDING && next_connect_state_ != CONNECT_STATE_NONE);

  return rv;
}

int TCPClientSocket::DoConnect() {
  CR_DCHECK(current_address_index_ >= 0);
  CR_DCHECK(current_address_index_ < static_cast<int>(addresses_.size()));

  const IPEndPoint& endpoint = addresses_[current_address_index_];

  if (previously_disconnected_) {
    was_ever_used_ = false;
    connection_attempts_.clear();
    previously_disconnected_ = false;
  }

  next_connect_state_ = CONNECT_STATE_CONNECT_COMPLETE;

  if (socket_->IsValid()) {
    CR_DCHECK(bind_address_);
  } else {
    int result = OpenSocket(endpoint.GetFamily());
    if (result != OK)
      return result;

    if (bind_address_) {
      result = socket_->Bind(*bind_address_);
      if (result != OK) {
        socket_->Close();
        return result;
      }
    }
  }

  if (before_connect_callback_) {
    int result = before_connect_callback_.Run();
    CR_DCHECK(ERR_IO_PENDING != result);
    if (result != crnet::OK)
      return result;
  }

  // Notify |socket_performance_watcher_| only if the |socket_| is reused to
  // connect to a different IP Address.
  if (socket_->socket_performance_watcher() && current_address_index_ != 0)
    socket_->socket_performance_watcher()->OnConnectionChanged();

  return ConnectInternal(endpoint);
}

int TCPClientSocket::DoConnectComplete(int result) {
  if (result == OK)
    return OK;  // Done!

  connection_attempts_.push_back(
      ConnectionAttempt(addresses_[current_address_index_], result));

  // Don't try the next address if entering suspend mode.
  if (result == ERR_NETWORK_IO_SUSPENDED)
    return result;

  // Close whatever partially connected socket we currently have.
  DoDisconnect();

  // Try to fall back to the next address in the list.
  if (current_address_index_ + 1 < static_cast<int>(addresses_.size())) {
    next_connect_state_ = CONNECT_STATE_CONNECT;
    ++current_address_index_;
    return OK;
  }

  // Otherwise there is nothing to fall back to, so give up.
  return result;
}

int TCPClientSocket::ConnectInternal(const IPEndPoint& endpoint) {
  // |socket_| is owned by this class and the callback won't be run once
  // |socket_| is gone. Therefore, it is safe to use cr::Unretained() here.
  return socket_->Connect(endpoint,
                          cr::BindOnce(&TCPClientSocket::DidCompleteConnect,
                                       cr::Unretained(this)));
}

void TCPClientSocket::Disconnect() {
  DoDisconnect();
  current_address_index_ = -1;
  bind_address_.reset();

  // Cancel any pending callbacks. Not done in DoDisconnect() because that's
  // called on connection failure, when the connect callback will need to be
  // invoked.
  was_disconnected_on_suspend_ = false;
  connect_callback_.Reset();
  read_callback_.Reset();
  write_callback_.Reset();
}

void TCPClientSocket::DoDisconnect() {
  total_received_bytes_ = 0;

  // If connecting or already connected, record that the socket has been
  // disconnected.
  previously_disconnected_ = socket_->IsValid() && current_address_index_ >= 0;
  socket_->Close();

  // Invalidate weak pointers, so if in the middle of a callback in OnSuspend,
  // and something destroys this, no other callback is invoked.
  weak_ptr_factory_.InvalidateWeakPtrs();
}

bool TCPClientSocket::IsConnected() const {
  return socket_->IsConnected();
}

bool TCPClientSocket::IsConnectedAndIdle() const {
  return socket_->IsConnectedAndIdle();
}

int TCPClientSocket::GetPeerAddress(IPEndPoint* address) const {
  return socket_->GetPeerAddress(address);
}

int TCPClientSocket::GetLocalAddress(IPEndPoint* address) const {
  CR_DCHECK(address);

  if (!socket_->IsValid()) {
    if (bind_address_) {
      *address = *bind_address_;
      return OK;
    }
    return ERR_SOCKET_NOT_CONNECTED;
  }

  return socket_->GetLocalAddress(address);
}

bool TCPClientSocket::WasEverUsed() const {
  return was_ever_used_;
}

int TCPClientSocket::Read(cr::IOBuffer* buf,
                          int buf_len,
                          CompletionOnceCallback callback) {
  return ReadCommon(buf, buf_len, std::move(callback), /*read_if_ready=*/false);
}

int TCPClientSocket::ReadIfReady(cr::IOBuffer* buf,
                                 int buf_len,
                                 CompletionOnceCallback callback) {
  return ReadCommon(buf, buf_len, std::move(callback), /*read_if_ready=*/true);
}

int TCPClientSocket::CancelReadIfReady() {
  CR_DCHECK(read_callback_);
  read_callback_.Reset();
  return socket_->CancelReadIfReady();
}

int TCPClientSocket::Write(
    cr::IOBuffer* buf,
    int buf_len,
    CompletionOnceCallback callback) {
  CR_DCHECK(!callback.is_null());
  CR_DCHECK(write_callback_.is_null());

  if (was_disconnected_on_suspend_)
    return ERR_NETWORK_IO_SUSPENDED;

  // |socket_| is owned by this class and the callback won't be run once
  // |socket_| is gone. Therefore, it is safe to use base::Unretained() here.
  CompletionOnceCallback complete_write_callback = cr::BindOnce(
      &TCPClientSocket::DidCompleteWrite, cr::Unretained(this));
  int result = socket_->Write(buf, buf_len, std::move(complete_write_callback));
  if (result == ERR_IO_PENDING) {
    write_callback_ = std::move(callback);
  } else if (result > 0) {
    was_ever_used_ = true;
  }

  return result;
}

int TCPClientSocket::SetReceiveBufferSize(int32_t size) {
  return socket_->SetReceiveBufferSize(size);
}

int TCPClientSocket::SetSendBufferSize(int32_t size) {
  return socket_->SetSendBufferSize(size);
}

void TCPClientSocket::GetConnectionAttempts(ConnectionAttempts* out) const {
  *out = connection_attempts_;
}

void TCPClientSocket::ClearConnectionAttempts() {
  connection_attempts_.clear();
}

void TCPClientSocket::AddConnectionAttempts(
    const ConnectionAttempts& attempts) {
  connection_attempts_.insert(connection_attempts_.begin(), attempts.begin(),
                              attempts.end());
}

int64_t TCPClientSocket::GetTotalReceivedBytes() const {
  return total_received_bytes_;
}

#if defined(MINI_CHROMIUM_OS_POSIX)
void TCPClientSocket::ApplySocketTag(const SocketTag& tag) {
  socket_->ApplySocketTag(tag);
}
#endif

///void TCPClientSocket::OnSuspend() {
///#if defined(TCP_CLIENT_SOCKET_OBSERVES_SUSPEND)
///  // If the socket is connected, or connecting, act as if current and future
///  // operations on the socket fail with ERR_NETWORK_IO_SUSPENDED, until the
///  // socket is reconnected.
///
///  if (next_connect_state_ != CONNECT_STATE_NONE) {
///    socket_->Close();
///    DidCompleteConnect(ERR_NETWORK_IO_SUSPENDED);
///    return;
///  }
///
///  // Nothing to do. Use IsValid() rather than IsConnected() because it results
///  // in more testable code, as when calling OnSuspend mode on two sockets
///  // connected to each other will otherwise cause two sockets to behave
///  // differently from each other.
///  if (!socket_->IsValid())
///    return;
///
///  // Use Close() rather than Disconnect() / DoDisconnect() to avoid mutating
///  // state, which more closely matches normal read/write error behavior.
///  socket_->Close();
///
///  was_disconnected_on_suspend_ = true;
///
///  // Grab a weak pointer just in case calling read callback results in |this|
///  // being destroyed, or disconnected. In either case, should not run the write
///  // callback.
///  base::WeakPtr<TCPClientSocket> weak_this = weak_ptr_factory_.GetWeakPtr();
///
///  // Have to grab the write callback now, as it's theoretically possible for the
///  // read callback to reconnects the socket, that reconnection to complete
///  // synchronously, and then for it to start a new write. That also means this
///  // code can't use DidCompleteWrite().
///  CompletionOnceCallback write_callback = std::move(write_callback_);
///  if (read_callback_)
///    DidCompleteRead(ERR_NETWORK_IO_SUSPENDED);
///  if (weak_this && write_callback)
///    std::move(write_callback).Run(ERR_NETWORK_IO_SUSPENDED);
///#endif  // defined(TCP_CLIENT_SOCKET_OBSERVES_SUSPEND)
///}

void TCPClientSocket::DidCompleteConnect(int result) {
  CR_DCHECK(next_connect_state_ == CONNECT_STATE_CONNECT_COMPLETE);
  CR_DCHECK(result != ERR_IO_PENDING);
  CR_DCHECK(!connect_callback_.is_null());

  result = DoConnectLoop(result);
  if (result != ERR_IO_PENDING) {
    std::move(connect_callback_).Run(result);
  }
}

void TCPClientSocket::DidCompleteRead(int result) {
  CR_DCHECK(!read_callback_.is_null());

  if (result > 0)
    total_received_bytes_ += result;
  DidCompleteReadWrite(std::move(read_callback_), result);
}

void TCPClientSocket::DidCompleteWrite(int result) {
  CR_DCHECK(!write_callback_.is_null());

  DidCompleteReadWrite(std::move(write_callback_), result);
}

void TCPClientSocket::DidCompleteReadWrite(CompletionOnceCallback callback,
                                           int result) {
  if (result > 0)
    was_ever_used_ = true;
  std::move(callback).Run(result);
}

int TCPClientSocket::OpenSocket(AddressFamily family) {
  CR_DCHECK(!socket_->IsValid());

  int result = socket_->Open(family);
  if (result != OK)
    return result;

  socket_->SetDefaultOptionsForClient();

  return OK;
}

}  // namespace crnet
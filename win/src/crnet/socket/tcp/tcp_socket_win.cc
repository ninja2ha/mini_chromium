// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/socket/tcp/tcp_socket.h"
#include "crnet/socket/tcp/tcp_socket_win.h"

#include <errno.h>
#include <mstcpip.h>

#include <utility>

#include "crbase/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/callback_helpers.h"
#include "crbase/buffer/io_buffer.h"
#include "crnet/base/address_list.h"
#include "crnet/base/ip_endpoint.h"
#include "crnet/base/net_errors.h"
///#include "crnet/base/network_activity_monitor.h"
///#include "crnet/base/network_change_notifier.h"
#include "crnet/base/sockaddr_storage.h"
#include "crnet/base/winsock_init.h"
#include "crnet/base/winsock_util.h"
#include "crnet/socket/socket_descriptor.h"
#include "crnet/socket/socket_options.h"
///#include "net/socket/socket_tag.h"

namespace crnet {

namespace {

const int kTCPKeepAliveSeconds = 45;

// Disable Nagle.
// Enable TCP Keep-Alive to prevent NAT routers from timing out TCP
// connections. See http://crbug.com/27400 for details.
bool SetTCPKeepAlive(SOCKET socket, BOOL enable, int delay_secs) {
  unsigned delay = delay_secs * 1000;
  struct tcp_keepalive keepalive_vals = {
      enable ? 1u : 0u,  // TCP keep-alive on.
      delay,  // Delay seconds before sending first TCP keep-alive packet.
      delay,  // Delay seconds between sending TCP keep-alive packets.
  };
  DWORD bytes_returned = 0xABAB;
  int rv = WSAIoctl(socket, SIO_KEEPALIVE_VALS, &keepalive_vals,
                    sizeof(keepalive_vals), nullptr, 0, &bytes_returned,
                    nullptr, nullptr);
  int os_error = WSAGetLastError();
  CR_DCHECK(!rv) << "Could not enable TCP Keep-Alive for socket: " << socket
                 << " [error: " << os_error << "].";

  // Disregard any failure in disabling nagle or enabling TCP Keep-Alive.
  return rv == 0;
}

int MapConnectError(int os_error) {
  switch (os_error) {
    // connect fails with WSAEACCES when Windows Firewall blocks the
    // connection.
    case WSAEACCES:
      return ERR_NETWORK_ACCESS_DENIED;
    case WSAETIMEDOUT:
      return ERR_CONNECTION_TIMED_OUT;
    default: {
      int net_error = MapSystemError(os_error);
      if (net_error == ERR_FAILED)
        return ERR_CONNECTION_FAILED;  // More specific than ERR_FAILED.

      // Give a more specific error when the user is offline.
      ///if (net_error == ERR_ADDRESS_UNREACHABLE &&
      ///    NetworkChangeNotifier::IsOffline()) {
      ///  return ERR_INTERNET_DISCONNECTED;
      ///}

      return net_error;
    }
  }
}

bool SetNonBlocking(SocketDescriptor fd) {
  unsigned long nonblocking = 1;
  if (ioctlsocket(fd, FIONBIO, &nonblocking) == 0)
    return true;
  return false;
}

bool SetNonBlockingAndGetError(SocketDescriptor fd, int* os_error) {
  bool ret = SetNonBlocking(fd);
  *os_error = WSAGetLastError();
  return ret;
}

}  // namespace

//-----------------------------------------------------------------------------

// This class encapsulates all the state that has to be preserved as long as
// there is a network IO operation in progress. If the owner TCPSocketWin is
// destroyed while an operation is in progress, the Core is detached and it
// lives until the operation completes and the OS doesn't reference any resource
// declared on this class anymore.
class TCPSocketWin::Core : public cr::RefCounted<Core> {
 public:
  Core(const Core&) = delete;
  Core& operator=(const Core&) = delete;

  explicit Core(TCPSocketWin* socket);

  // Start watching for the end of a read or write operation.
  void WatchForRead();
  void WatchForWrite();

  // Stops watching for read.
  void StopWatchingForRead();

  // The TCPSocketWin is going away.
  void Detach();

  // Event handle for monitoring connect and read events through WSAEventSelect.
  HANDLE read_event_;

  // OVERLAPPED variable for overlapped writes.
  // TODO(mmenke): Can writes be switched to WSAEventSelect as well? That would
  // allow removing this class. The only concern is whether that would have a
  // negative perf impact.
  OVERLAPPED write_overlapped_;

  // The buffers used in Read() and Write().
  cr::RefPtr<cr::IOBuffer> read_iobuffer_;
  cr::RefPtr<cr::IOBuffer> write_iobuffer_;
  int read_buffer_length_;
  int write_buffer_length_;

  bool non_blocking_reads_initialized_;

 private:
  friend class cr::RefCounted<Core>;

  class ReadDelegate : public cr::win::ObjectWatcher::Delegate {
   public:
    explicit ReadDelegate(Core* core) : core_(core) {}
    ~ReadDelegate() override {}

    // base::ObjectWatcher::Delegate methods:
    void OnObjectSignaled(HANDLE object) override;

   private:
    Core* const core_;
  };

  class WriteDelegate : public cr::win::ObjectWatcher::Delegate {
   public:
    explicit WriteDelegate(Core* core) : core_(core) {}
    ~WriteDelegate() override {}

    // base::ObjectWatcher::Delegate methods:
    void OnObjectSignaled(HANDLE object) override;

   private:
    Core* const core_;
  };

  ~Core();

  // The socket that created this object.
  TCPSocketWin* socket_;

  // |reader_| handles the signals from |read_watcher_|.
  ReadDelegate reader_;
  // |writer_| handles the signals from |write_watcher_|.
  WriteDelegate writer_;

  // |read_watcher_| watches for events from Connect() and Read().
  cr::win::ObjectWatcher read_watcher_;
  // |write_watcher_| watches for events from Write();
  cr::win::ObjectWatcher write_watcher_;
};

TCPSocketWin::Core::Core(TCPSocketWin* socket)
    : read_event_(WSACreateEvent()),
      read_buffer_length_(0),
      write_buffer_length_(0),
      non_blocking_reads_initialized_(false),
      socket_(socket),
      reader_(this),
      writer_(this) {
  memset(&write_overlapped_, 0, sizeof(write_overlapped_));
  write_overlapped_.hEvent = WSACreateEvent();
}

TCPSocketWin::Core::~Core() {
  // Detach should already have been called.
  CR_DCHECK(!socket_);

  // Stop the write watcher.  The read watcher should already have been stopped
  // in Detach().
  write_watcher_.StopWatching();
  WSACloseEvent(write_overlapped_.hEvent);
  memset(&write_overlapped_, 0xaf, sizeof(write_overlapped_));
}

void TCPSocketWin::Core::WatchForRead() {
  // Reads use WSAEventSelect, which closesocket() cancels so unlike writes,
  // there's no need to increment the reference count here.
  read_watcher_.StartWatchingOnce(read_event_, &reader_, CR_FROM_HERE);
}

void TCPSocketWin::Core::WatchForWrite() {
  // We grab an extra reference because there is an IO operation in progress.
  // Balanced in WriteDelegate::OnObjectSignaled().
  AddRef();
  write_watcher_.StartWatchingOnce(write_overlapped_.hEvent, &writer_, 
                                   CR_FROM_HERE);
}

void TCPSocketWin::Core::StopWatchingForRead() {
  CR_DCHECK(!socket_->waiting_connect_);

  read_watcher_.StopWatching();
}

void TCPSocketWin::Core::Detach() {
  // Stop watching the read watcher. A read won't be signalled after the Detach
  // call, since the socket has been closed, but it's possible the event was
  // signalled when the socket was closed, but hasn't been handled yet, so need
  // to stop watching now to avoid trying to handle the event. See
  // https://crbug.com/831149
  read_watcher_.StopWatching();
  WSACloseEvent(read_event_);

  socket_ = nullptr;
}

void TCPSocketWin::Core::ReadDelegate::OnObjectSignaled(HANDLE object) {
  CR_DCHECK(object == core_->read_event_);
  CR_DCHECK(core_->socket_);
  if (core_->socket_->waiting_connect_)
    core_->socket_->DidCompleteConnect();
  else
    core_->socket_->DidSignalRead();
}

void TCPSocketWin::Core::WriteDelegate::OnObjectSignaled(
    HANDLE object) {
  CR_DCHECK(object == core_->write_overlapped_.hEvent);
  if (core_->socket_)
    core_->socket_->DidCompleteWrite();

  // Matches the AddRef() in WatchForWrite().
  core_->Release();
}

//-----------------------------------------------------------------------------

TCPSocketWin::TCPSocketWin(
    std::unique_ptr<SocketPerformanceWatcher> socket_performance_watcher)
    : socket_(INVALID_SOCKET),
      socket_performance_watcher_(std::move(socket_performance_watcher)),
      accept_event_(WSA_INVALID_EVENT),
      accept_socket_(nullptr),
      accept_address_(nullptr),
      waiting_connect_(false),
      waiting_read_(false),
      waiting_write_(false),
      connect_os_error_(0),
      logging_multiple_connect_attempts_(false) {
  EnsureWinsockInit();
}

TCPSocketWin::~TCPSocketWin() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  Close();
}

int TCPSocketWin::Open(AddressFamily family) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket_ == INVALID_SOCKET);

  socket_ = CreatePlatformSocket(ConvertAddressFamily(family), SOCK_STREAM,
                                 IPPROTO_TCP);
  int os_error = WSAGetLastError();
  if (socket_ == INVALID_SOCKET) {
    CR_PLOG(Error) << "CreatePlatformSocket() returned an error";
    return MapSystemError(os_error);
  }

  if (!SetNonBlockingAndGetError(socket_, &os_error)) {
    int result = MapSystemError(os_error);
    Close();
    return result;
  }

  return OK;
}

int TCPSocketWin::AdoptConnectedSocket(SocketDescriptor socket,
                                       const IPEndPoint& peer_address) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket_ == INVALID_SOCKET);
  CR_DCHECK(!core_.get());

  socket_ = socket;

  int os_error;
  if (!SetNonBlockingAndGetError(socket_, &os_error)) {
    int result = MapSystemError(os_error);
    Close();
    return result;
  }

  core_ = new Core(this);
  peer_address_.reset(new IPEndPoint(peer_address));

  return OK;
}

int TCPSocketWin::AdoptUnconnectedSocket(SocketDescriptor socket) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket_ == INVALID_SOCKET);

  socket_ = socket;

  int os_error;
  if (!SetNonBlockingAndGetError(socket_, &os_error)) {
    int result = MapSystemError(os_error);
    Close();
    return result;
  }

  // |core_| is not needed for sockets that are used to accept connections.
  // The operation here is more like Open but with an existing socket.

  return OK;
}

int TCPSocketWin::Bind(const IPEndPoint& address) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket_ != INVALID_SOCKET);

  SockaddrStorage storage;
  if (!address.ToSockAddr(storage.addr(), &storage.addr_len))
    return ERR_ADDRESS_INVALID;

  int result = bind(socket_, storage.addr(), storage.addr_len);
  int os_error = WSAGetLastError();
  if (result < 0) {
    CR_PLOG(Error) << "bind() returned an error";
    return MapSystemError(os_error);
  }

  return OK;
}

int TCPSocketWin::Listen(int backlog) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(backlog > 0);
  CR_DCHECK(socket_ != INVALID_SOCKET);
  CR_DCHECK(accept_event_ == WSA_INVALID_EVENT);

  accept_event_ = WSACreateEvent();
  int os_error = WSAGetLastError();
  if (accept_event_ == WSA_INVALID_EVENT) {
    CR_PLOG(Error) << "WSACreateEvent()";
    return MapSystemError(os_error);
  }

  int result = listen(socket_, backlog);
  os_error = WSAGetLastError();
  if (result < 0) {
    CR_PLOG(Error) << "listen() returned an error";
    return MapSystemError(os_error);
  }

  return OK;
}

int TCPSocketWin::Accept(std::unique_ptr<TCPSocketWin>* socket,
                         IPEndPoint* address,
                         CompletionOnceCallback callback) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket);
  CR_DCHECK(address);
  CR_DCHECK(!callback.is_null());
  CR_DCHECK(accept_callback_.is_null());

  int result = AcceptInternal(socket, address);

  if (result == ERR_IO_PENDING) {
    // Start watching.
    WSAEventSelect(socket_, accept_event_, FD_ACCEPT);
    accept_watcher_.StartWatchingOnce(accept_event_, this, CR_FROM_HERE);

    accept_socket_ = socket;
    accept_address_ = address;
    accept_callback_ = std::move(callback);
  }

  return result;
}

int TCPSocketWin::Connect(const IPEndPoint& address,
                          CompletionOnceCallback callback) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket_ != INVALID_SOCKET);
  CR_DCHECK(!waiting_connect_);

  // |peer_address_| and |core_| will be non-NULL if Connect() has been called.
  // Unless Close() is called to reset the internal state, a second call to
  // Connect() is not allowed.
  // Please note that we enforce this even if the previous Connect() has
  // completed and failed. Although it is allowed to connect the same |socket_|
  // again after a connection attempt failed on Windows, it results in
  // unspecified behavior according to POSIX. Therefore, we make it behave in
  // the same way as TCPSocketPosix.
  CR_DCHECK(!peer_address_ && !core_.get());

  ///if (!logging_multiple_connect_attempts_)
  ///  LogConnectBegin(AddressList(address));

  peer_address_.reset(new IPEndPoint(address));

  int rv = DoConnect();
  if (rv == ERR_IO_PENDING) {
    // Synchronous operation not supported.
    CR_DCHECK(!callback.is_null());
    read_callback_ = std::move(callback);
    waiting_connect_ = true;
  } else {
    DoConnectComplete(rv);
  }

  return rv;
}

bool TCPSocketWin::IsConnected() const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (socket_ == INVALID_SOCKET || waiting_connect_)
    return false;

  if (waiting_read_)
    return true;

  // Check if connection is alive.
  char c;
  int rv = recv(socket_, &c, 1, MSG_PEEK);
  int os_error = WSAGetLastError();
  if (rv == 0)
    return false;
  if (rv == SOCKET_ERROR && os_error != WSAEWOULDBLOCK)
    return false;

  return true;
}

bool TCPSocketWin::IsConnectedAndIdle() const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (socket_ == INVALID_SOCKET || waiting_connect_)
    return false;

  if (waiting_read_)
    return true;

  // Check if connection is alive and we haven't received any data
  // unexpectedly.
  char c;
  int rv = recv(socket_, &c, 1, MSG_PEEK);
  int os_error = WSAGetLastError();
  if (rv >= 0)
    return false;
  if (os_error != WSAEWOULDBLOCK)
    return false;

  return true;
}

int TCPSocketWin::Read(cr::IOBuffer* buf,
                       int buf_len,
                       CompletionOnceCallback callback) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(!core_->read_iobuffer_.get());
  // cr::Unretained() is safe because RetryRead() won't be called when |this|
  // is gone.
  int rv =
      ReadIfReady(buf, buf_len,
                  cr::BindOnce(&TCPSocketWin::RetryRead, cr::Unretained(this)));
  if (rv != ERR_IO_PENDING)
    return rv;
  read_callback_ = std::move(callback);
  core_->read_iobuffer_ = buf;
  core_->read_buffer_length_ = buf_len;
  return ERR_IO_PENDING;
}

int TCPSocketWin::ReadIfReady(cr::IOBuffer* buf,
                              int buf_len,
                              CompletionOnceCallback callback) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket_ != INVALID_SOCKET);
  CR_DCHECK(!waiting_read_);
  CR_DCHECK(read_if_ready_callback_.is_null());

  if (!core_->non_blocking_reads_initialized_) {
    WSAEventSelect(socket_, core_->read_event_, FD_READ | FD_CLOSE);
    core_->non_blocking_reads_initialized_ = true;
  }
  int rv = recv(socket_, buf->data(), buf_len, 0);
  int os_error = WSAGetLastError();
  if (rv == SOCKET_ERROR) {
    if (os_error != WSAEWOULDBLOCK) {
      int net_error = MapSystemError(os_error);
      return net_error;
    }
  } else {
    ///net_log_.AddByteTransferEvent(NetLogEventType::SOCKET_BYTES_RECEIVED, rv,
    ///                              buf->data());
    ///NetworkActivityMonitor::GetInstance()->IncrementBytesReceived(rv);
    return rv;
  }

  waiting_read_ = true;
  read_if_ready_callback_ = std::move(callback);
  core_->WatchForRead();
  return ERR_IO_PENDING;
}

int TCPSocketWin::CancelReadIfReady() {
  CR_DCHECK(read_callback_.is_null());
  CR_DCHECK(!read_if_ready_callback_.is_null());
  CR_DCHECK(waiting_read_);

  core_->StopWatchingForRead();
  read_if_ready_callback_.Reset();
  waiting_read_ = false;
  return crnet::OK;
}

int TCPSocketWin::Write(
    cr::IOBuffer* buf,
    int buf_len,
    CompletionOnceCallback callback) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(socket_ != INVALID_SOCKET);
  CR_DCHECK(!waiting_write_);
  CR_CHECK(write_callback_.is_null());
  CR_DCHECK(buf_len > 0);
  CR_DCHECK(!core_->write_iobuffer_.get());

  WSABUF write_buffer;
  write_buffer.len = buf_len;
  write_buffer.buf = buf->data();

  // TODO(wtc): Remove the assertion after enough testing.
  AssertEventNotSignaled(core_->write_overlapped_.hEvent);
  DWORD num;
  int rv = WSASend(socket_, &write_buffer, 1, &num, 0,
                   &core_->write_overlapped_, nullptr);
  int os_error = WSAGetLastError();
  if (rv == 0) {
    if (ResetEventIfSignaled(core_->write_overlapped_.hEvent)) {
      rv = static_cast<int>(num);
      if (rv > buf_len || rv < 0) {
        // It seems that some winsock interceptors report that more was written
        // than was available. Treat this as an error.  http://crbug.com/27870
        CR_LOG(Error) << "Detected broken LSP: Asked to write " << buf_len
                      << " bytes, but " << rv << " bytes reported.";
        return ERR_WINSOCK_UNEXPECTED_WRITTEN_BYTES;
      }
      ///NetworkActivityMonitor::GetInstance()->IncrementBytesSent(rv);
      return rv;
    }
  } else {
    if (os_error != WSA_IO_PENDING) {
      int net_error = MapSystemError(os_error);
      return net_error;
    }
  }
  waiting_write_ = true;
  write_callback_ = std::move(callback);
  core_->write_iobuffer_ = buf;
  core_->write_buffer_length_ = buf_len;
  core_->WatchForWrite();
  return ERR_IO_PENDING;
}

int TCPSocketWin::GetLocalAddress(IPEndPoint* address) const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(address);

  SockaddrStorage storage;
  if (getsockname(socket_, storage.addr(), &storage.addr_len)) {
    int os_error = WSAGetLastError();
    return MapSystemError(os_error);
  }
  if (!address->FromSockAddr(storage.addr(), storage.addr_len))
    return ERR_ADDRESS_INVALID;

  return OK;
}

int TCPSocketWin::GetPeerAddress(IPEndPoint* address) const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  CR_DCHECK(address);
  if (!IsConnected())
    return ERR_SOCKET_NOT_CONNECTED;
  *address = *peer_address_;
  return OK;
}

int TCPSocketWin::SetDefaultOptionsForServer() {
  return SetExclusiveAddrUse();
}

void TCPSocketWin::SetDefaultOptionsForClient() {
  SetTCPNoDelay(socket_, /*no_delay=*/true);
  SetTCPKeepAlive(socket_, true, kTCPKeepAliveSeconds);
}

int TCPSocketWin::SetExclusiveAddrUse() {
  // On Windows, a bound end point can be hijacked by another process by
  // setting SO_REUSEADDR. Therefore a Windows-only option SO_EXCLUSIVEADDRUSE
  // was introduced in Windows NT 4.0 SP4. If the socket that is bound to the
  // end point has SO_EXCLUSIVEADDRUSE enabled, it is not possible for another
  // socket to forcibly bind to the end point until the end point is unbound.
  // It is recommend that all server applications must use SO_EXCLUSIVEADDRUSE.
  // MSDN: http://goo.gl/M6fjQ.
  //
  // Unlike on *nix, on Windows a TCP server socket can always bind to an end
  // point in TIME_WAIT state without setting SO_REUSEADDR, therefore it is not
  // needed here.
  //
  // SO_EXCLUSIVEADDRUSE will prevent a TCP client socket from binding to an end
  // point in TIME_WAIT status. It does not have this effect for a TCP server
  // socket.

  BOOL true_value = 1;
  int rv = setsockopt(socket_, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                      reinterpret_cast<const char*>(&true_value),
                      sizeof(true_value));
  if (rv < 0)
    return MapSystemError(errno);
  return OK;
}

int TCPSocketWin::SetReceiveBufferSize(int32_t size) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  return SetSocketReceiveBufferSize(socket_, size);
}

int TCPSocketWin::SetSendBufferSize(int32_t size) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  return SetSocketSendBufferSize(socket_, size);
}

bool TCPSocketWin::SetKeepAlive(bool enable, int delay) {
  return SetTCPKeepAlive(socket_, enable, delay);
}

bool TCPSocketWin::SetNoDelay(bool no_delay) {
  return SetTCPNoDelay(socket_, no_delay) == OK;
}

void TCPSocketWin::Close() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (socket_ != INVALID_SOCKET) {
    // Note: don't use CancelIo to cancel pending IO because it doesn't work
    // when there is a Winsock layered service provider.

    // In most socket implementations, closing a socket results in a graceful
    // connection shutdown, but in Winsock we have to call shutdown explicitly.
    // See the MSDN page "Graceful Shutdown, Linger Options, and Socket Closure"
    // at http://msdn.microsoft.com/en-us/library/ms738547.aspx
    shutdown(socket_, SD_SEND);

    // This cancels any pending IO.
    if (closesocket(socket_) < 0)
      CR_PLOG(Error) << "closesocket";
    socket_ = INVALID_SOCKET;
  }

  if (!accept_callback_.is_null()) {
    accept_watcher_.StopWatching();
    accept_socket_ = nullptr;
    accept_address_ = nullptr;
    accept_callback_.Reset();
  }

  if (accept_event_) {
    WSACloseEvent(accept_event_);
    accept_event_ = WSA_INVALID_EVENT;
  }

  if (core_.get()) {
    core_->Detach();
    core_ = nullptr;

    // |core_| may still exist and own a reference to itself, if there's a
    // pending write. It has to stay alive until the operation completes, even
    // when the socket is closed. This is not the case for reads.
  }

  waiting_connect_ = false;
  waiting_read_ = false;
  waiting_write_ = false;

  read_callback_.Reset();
  read_if_ready_callback_.Reset();
  write_callback_.Reset();
  peer_address_.reset();
  connect_os_error_ = 0;
}

void TCPSocketWin::DetachFromThread() {
  CR_DETACH_FROM_THREAD(thread_checker_);
}

int TCPSocketWin::AcceptInternal(std::unique_ptr<TCPSocketWin>* socket,
                                 IPEndPoint* address) {
  SockaddrStorage storage;
  SocketDescriptor new_socket = 
      accept(socket_, storage.addr(), &storage.addr_len);
  int os_error = WSAGetLastError();
  if (new_socket == kInvalidSocket) {
    int net_error = MapSystemError(os_error);
    return net_error;
  }

  IPEndPoint ip_end_point;
  if (!ip_end_point.FromSockAddr(storage.addr(), storage.addr_len)) {
    CR_NOTREACHED();
    if (closesocket(new_socket) < 0)
      CR_PLOG(Error) << "closesocket";
    int net_error = ERR_ADDRESS_INVALID;
    return net_error;
  }
  std::unique_ptr<TCPSocketWin> tcp_socket(
      new TCPSocketWin(nullptr));
  int adopt_result = tcp_socket->AdoptConnectedSocket(new_socket, ip_end_point);
  if (adopt_result != OK) {
    return adopt_result;
  }
  *socket = std::move(tcp_socket);
  *address = ip_end_point;
  return OK;
}

void TCPSocketWin::OnObjectSignaled(HANDLE object) {
  WSANETWORKEVENTS ev;
  if (WSAEnumNetworkEvents(socket_, accept_event_, &ev) == SOCKET_ERROR) {
    CR_PLOG(Error) << "WSAEnumNetworkEvents()";
    return;
  }

  if (ev.lNetworkEvents & FD_ACCEPT) {
    int result = AcceptInternal(accept_socket_, accept_address_);
    if (result != ERR_IO_PENDING) {
      accept_socket_ = nullptr;
      accept_address_ = nullptr;
      std::move(accept_callback_).Run(result);
    }
  } else {
    // This happens when a client opens a connection and closes it before we
    // have a chance to accept it.
    CR_DCHECK(ev.lNetworkEvents == 0);

    // Start watching the next FD_ACCEPT event.
    WSAEventSelect(socket_, accept_event_, FD_ACCEPT);
    accept_watcher_.StartWatchingOnce(accept_event_, this, CR_FROM_HERE);
  }
}

int TCPSocketWin::DoConnect() {
  CR_DCHECK(connect_os_error_ == 0);
  CR_DCHECK(!core_.get());

  core_ = new Core(this);

  // WSAEventSelect sets the socket to non-blocking mode as a side effect.
  // Our connect() and recv() calls require that the socket be non-blocking.
  WSAEventSelect(socket_, core_->read_event_, FD_CONNECT);

  SockaddrStorage storage;
  if (!peer_address_->ToSockAddr(storage.addr(), &storage.addr_len))
    return ERR_ADDRESS_INVALID;

  if (!connect(socket_, storage.addr(), storage.addr_len)) {
    // Connected without waiting!
    //
    // The MSDN page for connect says:
    //   With a nonblocking socket, the connection attempt cannot be completed
    //   immediately. In this case, connect will return SOCKET_ERROR, and
    //   WSAGetLastError will return WSAEWOULDBLOCK.
    // which implies that for a nonblocking socket, connect never returns 0.
    // It's not documented whether the event object will be signaled or not
    // if connect does return 0.  So the code below is essentially dead code
    // and we don't know if it's correct.
    CR_NOTREACHED();

    if (ResetEventIfSignaled(core_->read_event_))
      return OK;
  } else {
    int os_error = WSAGetLastError();
    if (os_error != WSAEWOULDBLOCK) {
      CR_LOG(Error) << "connect failed: " << os_error;
      connect_os_error_ = os_error;
      int rv = MapConnectError(os_error);
      CR_CHECK(ERR_IO_PENDING != rv);
      return rv;
    }
  }

  core_->WatchForRead();
  return ERR_IO_PENDING;
}

void TCPSocketWin::DoConnectComplete(int result) {
  // Log the end of this attempt (and any OS error it threw).
  int os_error = connect_os_error_;
  connect_os_error_ = 0;
}

void TCPSocketWin::RetryRead(int rv) {
  CR_DCHECK(core_->read_iobuffer_);

  if (rv == OK) {
    // cr::Unretained() is safe because RetryRead() won't be called when
    // |this| is gone.
    rv = ReadIfReady(
        core_->read_iobuffer_.get(), core_->read_buffer_length_,
        cr::BindOnce(&TCPSocketWin::RetryRead, cr::Unretained(this)));
    if (rv == ERR_IO_PENDING)
      return;
  }
  core_->read_iobuffer_ = nullptr;
  core_->read_buffer_length_ = 0;
  std::move(read_callback_).Run(rv);
}

void TCPSocketWin::DidCompleteConnect() {
  CR_DCHECK(waiting_connect_);
  CR_DCHECK(!read_callback_.is_null());
  int result;

  WSANETWORKEVENTS events;
  int rv = WSAEnumNetworkEvents(socket_, core_->read_event_, &events);
  int os_error = WSAGetLastError();
  if (rv == SOCKET_ERROR) {
    CR_NOTREACHED();
    result = MapSystemError(os_error);
  } else if (events.lNetworkEvents & FD_CONNECT) {
    os_error = events.iErrorCode[FD_CONNECT_BIT];
    result = MapConnectError(os_error);
  } else {
    CR_NOTREACHED();
    result = ERR_UNEXPECTED;
  }

  connect_os_error_ = os_error;
  DoConnectComplete(result);
  waiting_connect_ = false;

  CR_DCHECK(result != ERR_IO_PENDING);
  std::move(read_callback_).Run(result);
}

void TCPSocketWin::DidCompleteWrite() {
  CR_DCHECK(waiting_write_);
  CR_DCHECK(!write_callback_.is_null());

  DWORD num_bytes, flags;
  BOOL ok = WSAGetOverlappedResult(socket_, &core_->write_overlapped_,
                                   &num_bytes, FALSE, &flags);
  int os_error = WSAGetLastError();
  WSAResetEvent(core_->write_overlapped_.hEvent);
  waiting_write_ = false;
  int rv;
  if (!ok) {
    rv = MapSystemError(os_error);
  } else {
    rv = static_cast<int>(num_bytes);
    if (rv > core_->write_buffer_length_ || rv < 0) {
      // It seems that some winsock interceptors report that more was written
      // than was available. Treat this as an error.  http://crbug.com/27870
      CR_LOG(Error) << "Detected broken LSP: Asked to write "
                    << core_->write_buffer_length_ << " bytes, but " << rv
                    << " bytes reported.";
      rv = ERR_WINSOCK_UNEXPECTED_WRITTEN_BYTES;
    } else {
      ///NetworkActivityMonitor::GetInstance()->IncrementBytesSent(num_bytes);
    }
  }

  core_->write_iobuffer_ = nullptr;

  CR_DCHECK(rv != ERR_IO_PENDING);
  std::move(write_callback_).Run(rv);
}

void TCPSocketWin::DidSignalRead() {
  CR_DCHECK(waiting_read_);
  CR_DCHECK(!read_if_ready_callback_.is_null());

  int os_error = 0;
  WSANETWORKEVENTS network_events;
  int rv = WSAEnumNetworkEvents(socket_, core_->read_event_, &network_events);
  os_error = WSAGetLastError();

  if (rv == SOCKET_ERROR) {
    rv = MapSystemError(os_error);
  } else if (network_events.lNetworkEvents) {
    CR_DCHECK((network_events.lNetworkEvents & ~(FD_READ | FD_CLOSE)) == 0);
    // If network_events.lNetworkEvents is FD_CLOSE and
    // network_events.iErrorCode[FD_CLOSE_BIT] is 0, it is a graceful
    // connection closure. It is tempting to directly set rv to 0 in
    // this case, but the MSDN pages for WSAEventSelect and
    // WSAAsyncSelect recommend we still call RetryRead():
    //   FD_CLOSE should only be posted after all data is read from a
    //   socket, but an application should check for remaining data upon
    //   receipt of FD_CLOSE to avoid any possibility of losing data.
    //
    // If network_events.iErrorCode[FD_READ_BIT] or
    // network_events.iErrorCode[FD_CLOSE_BIT] is nonzero, still call
    // RetryRead() because recv() reports a more accurate error code
    // (WSAECONNRESET vs. WSAECONNABORTED) when the connection was
    // reset.
    rv = OK;
  } else {
    // This may happen because Read() may succeed synchronously and
    // consume all the received data without resetting the event object.
    core_->WatchForRead();
    return;
  }

  CR_DCHECK(rv != ERR_IO_PENDING);
  waiting_read_ = false;
  std::move(read_if_ready_callback_).Run(rv);
}

bool TCPSocketWin::GetEstimatedRoundTripTime(cr::TimeDelta* out_rtt) const {
  CR_DCHECK(out_rtt);
  // TODO(bmcquade): Consider implementing using
  // GetPerTcpConnectionEStats/GetPerTcp6ConnectionEStats.
  return false;
}

}  // namespace crnet
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/channel.h"

#include <stdint.h>

#include <algorithm>
#include <limits>
#include <memory>

#include "crbase/logging/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/containers/queue.h"
#include "crbase/location.h"
#include "crbase/synchronization/lock.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/win/scoped_handle.h"
#include "crbase/win/win_util.h"
#include "crbase_runtime/message_pump/message_pump_for_io.h"
#include "crbase_runtime/task/current_thread.h"
#include "crbase_runtime/task_runner.h"

namespace mojo {
namespace core {

namespace {

class ChannelWinMessageQueue {
 public:
  explicit ChannelWinMessageQueue() {}
  ~ChannelWinMessageQueue() {}

  void Append(cr::RefPtr<cr::IOBuffer> message) {
    queue_.emplace_back(std::move(message));
  }

  cr::RefPtr<cr::IOBuffer> GetFirst() const { return queue_.front(); }

  cr::RefPtr<cr::IOBuffer> TakeFirst() {
    cr::RefPtr<cr::IOBuffer> message = std::move(queue_.front());
    queue_.pop_front();
    return message;
  }

  bool IsEmpty() const { return queue_.empty(); }

 private:
  cr::circular_deque<cr::RefPtr<cr::IOBuffer>> queue_;
};

class ChannelWin : public Channel,
                   public cr::CurrentThread::DestructionObserver,
                   public cr::MessagePumpForIO::IOHandler {
 public:
  ChannelWin(Delegate* delegate,
             ConnectionParams connection_params,
             cr::RefPtr<cr::SingleThreadTaskRunner> io_task_runner)
      : Channel(delegate),
        cr::MessagePumpForIO::IOHandler(CR_FROM_HERE),
        self_(this),
        io_task_runner_(io_task_runner) {
    if (connection_params.server_endpoint().is_valid()) {
      handle_ = connection_params.TakeServerEndpoint()
                    .TakePlatformHandle()
                    .TakeHandle();
      needs_connection_ = true;
    } else {
      handle_ =
          connection_params.TakeEndpoint().TakePlatformHandle().TakeHandle();
    }

    CR_CHECK(handle_.IsValid());
  }

  void Start() override {
    io_task_runner_->PostTask(
        CR_FROM_HERE, cr::BindOnce(&ChannelWin::StartOnIOThread, this));
  }

  void ShutDownImpl() override {
    // Always shut down asynchronously when called through the public interface.
    io_task_runner_->PostTask(
        CR_FROM_HERE, cr::BindOnce(&ChannelWin::ShutDownOnIOThread, this));
  }

  void Write(cr::RefPtr<cr::IOBuffer> message) override {
    bool write_error = false;
    {
      cr::AutoLock lock(write_lock_);
      if (reject_writes_)
        return;

      bool write_now = !delay_writes_ && outgoing_messages_.IsEmpty();
      outgoing_messages_.Append(std::move(message));
      if (write_now && !WriteNoLock(outgoing_messages_.GetFirst()))
        reject_writes_ = write_error = true;
    }
    if (write_error) {
      // Do not synchronously invoke OnWriteError(). Write() may have been
      // called by the delegate and we don't want to re-enter it.
      io_task_runner_->PostTask(CR_FROM_HERE,
                                cr::BindOnce(&ChannelWin::OnWriteError, this,
                                             Error::kDisconnected));
    }
  }

  void LeakHandle() override {
    CR_DCHECK(io_task_runner_->RunsTasksInCurrentSequence());
    leak_handle_ = true;
  }

 private:
  // May run on any thread.
  ~ChannelWin() override = default;

  void StartOnIOThread() {
    cr::CurrentThread::Get()->AddDestructionObserver(this);
    cr::CurrentIOThread::Get()->RegisterIOHandler(handle_.Get(), this);

    if (needs_connection_) {
      BOOL ok = ::ConnectNamedPipe(handle_.Get(), &connect_context_.overlapped);
      if (ok) {
        CR_PLOG(Error) << "Unexpected success while waiting for pipe connection";
        OnError(Error::kConnectionFailed);
        return;
      }

      const DWORD err = GetLastError();
      switch (err) {
        case ERROR_PIPE_CONNECTED:
          break;
        case ERROR_IO_PENDING:
          is_connect_pending_ = true;
          AddRef();
          return;
        case ERROR_NO_DATA:
        default:
          OnError(Error::kConnectionFailed);
          return;
      }
    }

    // Now that we have registered our IOHandler, we can start writing.
    {
      cr::AutoLock lock(write_lock_);
      if (delay_writes_) {
        delay_writes_ = false;
        WriteNextNoLock();
      }
    }

    // Keep this alive in case we synchronously run shutdown, via OnError(),
    // as a result of a ReadFile() failure on the channel.
    cr::RefPtr<ChannelWin> keep_alive(this);
    ReadMore(0);
  }

  void ShutDownOnIOThread() {
    cr::CurrentThread::Get()->RemoveDestructionObserver(this);

    // TODO(https://crbug.com/583525): This function is expected to be called
    // once, and |handle_| should be valid at this point.
    CR_CHECK(handle_.IsValid());
    CancelIo(handle_.Get());
    if (leak_handle_)
      cr::ignore_result(handle_.Take());
    else
      handle_.Close();

    // Allow |this| to be destroyed as soon as no IO is pending.
    self_ = nullptr;
  }

  // base::CurrentThread::DestructionObserver:
  void WillDestroyCurrentMessageLoop() override {
    CR_DCHECK(io_task_runner_->RunsTasksInCurrentSequence());
    if (self_)
      ShutDownOnIOThread();
  }

  // base::MessageLoop::IOHandler:
  void OnIOCompleted(cr::MessagePumpForIO::IOContext* context,
                     DWORD bytes_transfered,
                     DWORD error) override {
    if (error != ERROR_SUCCESS) {
      if (context == &write_context_) {
        {
          cr::AutoLock lock(write_lock_);
          reject_writes_ = true;
        }
        OnWriteError(Error::kDisconnected);
      } else {
        OnError(Error::kDisconnected);
      }
    } else if (context == &connect_context_) {
      CR_DCHECK(is_connect_pending_);
      is_connect_pending_ = false;
      ReadMore(0);

      cr::AutoLock lock(write_lock_);
      if (delay_writes_) {
        delay_writes_ = false;
        WriteNextNoLock();
      }
    } else if (context == &read_context_) {
      OnReadDone(static_cast<size_t>(bytes_transfered));
    } else {
      CR_CHECK(context == &write_context_);
      OnWriteDone(static_cast<size_t>(bytes_transfered));
    }
    Release();
  }

  void OnReadDone(size_t bytes_read) {
    CR_DCHECK(is_read_pending_);
    is_read_pending_ = false;

    if (bytes_read > 0) {
      size_t next_read_size = 0;
      if (OnReadComplete(bytes_read, &next_read_size)) {
        ReadMore(next_read_size);
      } else {
        OnError(Error::kReceivedMalformedData);
      }
    } else if (bytes_read == 0) {
      OnError(Error::kDisconnected);
    }
  }

  void OnWriteDone(size_t bytes_written) {
    if (bytes_written == 0)
      return;

    bool write_error = false;
    {
      cr::AutoLock lock(write_lock_);

      CR_DCHECK(is_write_pending_);
      is_write_pending_ = false;
      CR_DCHECK(!outgoing_messages_.IsEmpty());

      cr::RefPtr<cr::IOBuffer> message = outgoing_messages_.TakeFirst();

      // Overlapped WriteFile() to a pipe should always fully complete.
      if (message->size() != bytes_written)
        reject_writes_ = write_error = true;
      else if (!WriteNextNoLock())
        reject_writes_ = write_error = true;
    }
    if (write_error)
      OnWriteError(Error::kDisconnected);
  }

  void ReadMore(size_t next_read_size_hint) {
    CR_DCHECK(!is_read_pending_);

    size_t buffer_capacity = next_read_size_hint;
    char* buffer = GetReadBuffer(&buffer_capacity);
    CR_DCHECK(buffer_capacity > 0u);

    BOOL ok =
        ::ReadFile(handle_.Get(), buffer, static_cast<DWORD>(buffer_capacity),
                   NULL, &read_context_.overlapped);
    if (ok || GetLastError() == ERROR_IO_PENDING) {
      is_read_pending_ = true;
      AddRef();
    } else {
      OnError(Error::kDisconnected);
    }
  }

  bool WriteNoLock(cr::RefPtr<cr::IOBuffer> message) {
    BOOL ok = WriteFile(handle_.Get(), message->data(),
                        static_cast<DWORD>(message->size()), NULL,
                        &write_context_.overlapped);
    if (ok || GetLastError() == ERROR_IO_PENDING) {
      is_write_pending_ = true;
      AddRef();
      return true;
    }
    return false;
  }

  bool WriteNextNoLock() {
    if (outgoing_messages_.IsEmpty())
      return true;
    return WriteNoLock(outgoing_messages_.GetFirst());
  }

  void OnWriteError(Error error) {
    CR_DCHECK(io_task_runner_->RunsTasksInCurrentSequence());
    CR_DCHECK(reject_writes_);

    if (error == Error::kDisconnected) {
      // If we can't write because the pipe is disconnected then continue
      // reading to fetch any in-flight messages, relying on end-of-stream to
      // signal the actual disconnection.
      if (is_read_pending_ || is_connect_pending_)
        return;
    }

    OnError(error);
  }

  // Keeps the Channel alive at least until explicit shutdown on the IO thread.
  cr::RefPtr<Channel> self_;

  // The pipe handle this Channel uses for communication.
  cr::win::ScopedHandle handle_;

  // Indicates whether |handle_| must wait for a connection.
  bool needs_connection_ = false;

  const cr::RefPtr<cr::SingleThreadTaskRunner> io_task_runner_;

  cr::MessagePumpForIO::IOContext connect_context_;
  cr::MessagePumpForIO::IOContext read_context_;
  bool is_connect_pending_ = false;
  bool is_read_pending_ = false;

  // Protects all fields potentially accessed on multiple threads via Write().
  cr::Lock write_lock_;
  cr::MessagePumpForIO::IOContext write_context_;
  ChannelWinMessageQueue outgoing_messages_;
  bool delay_writes_ = true;
  bool reject_writes_ = false;
  bool is_write_pending_ = false;

  bool leak_handle_ = false;
};

}  // namespace

// static
cr::RefPtr<Channel> Channel::Create(
    Delegate* delegate,
    ConnectionParams connection_params,
    cr::RefPtr<cr::SingleThreadTaskRunner> io_task_runner) {
  return new ChannelWin(delegate, std::move(connection_params), io_task_runner);
}

}  // namespace core
}  // namespace mojo
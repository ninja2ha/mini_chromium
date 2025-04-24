// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cripc/ipc_channel_win.h"

#include <windows.h>
#include <stddef.h>
#include <stdint.h>

#include "crbase/logging.h"
#include "crbase/auto_reset.h"
#include "crbase/functional/bind.h"
#include "crbase/process/process_handle.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/strings/stringprintf.h"
#include "crbase/win/scoped_handle.h"

#include "cripc/ipc_delegate.h"

namespace cripc {

ChannelWin::State::State(ChannelWin* channel) : is_pending(false) {
  memset(&context.overlapped, 0, sizeof(context.overlapped));
}

ChannelWin::State::~State() {
  static_assert(offsetof(ChannelWin::State, context) == 0,
                "ChannelWin::State should have context as its first data"
                "member.");
}

ChannelWin::ChannelWin(const ChannelHandle& channel_handle,
                       Mode mode,
                       Channel::Delegate* delegate)
    : ChannelReader(delegate),
      input_state_(this),
      output_state_(this),
      waiting_connect_(mode & MODE_SERVER_FLAG),
      processing_incoming_(false),
      connected_(false),
      weak_factory_(this) {
  CreatePipe(channel_handle, mode);
}

ChannelWin::~ChannelWin() {
  Close();
}

void ChannelWin::Close() {
  if (thread_check_.get())
    CR_DCHECK(thread_check_->CalledOnValidThread());

  if (input_state_.is_pending || output_state_.is_pending)
    CancelIo(pipe_.Get());

  // Closing the handle at this point prevents us from issuing more requests
  // form OnIOCompleted().
  if (pipe_.IsValid())
    pipe_.Close();

  // Make sure all IO has completed.
  while (input_state_.is_pending || output_state_.is_pending) {
    cr::MessageLoopForIO::current()->WaitForIOCompletion(INFINITE, this);
  }

  connected_ = false;

  while (!output_queue_.empty()) {
    OutputElement* element = output_queue_.front();
    output_queue_.pop();
    delete element;
  }
}

bool ChannelWin::Send(cr::RefPtr<cr::IOBuffer> message) {
  CR_DCHECK(thread_check_->CalledOnValidThread());
  if (!pipe_.IsValid())
    return false;

  if (!prelim_queue_.empty()) {
    prelim_queue_.push(message);
    return true;
  }
  
  if (!connected_) {
    prelim_queue_.push(message);
    return true;
  }

  return ProcessMessageForDelivery(message);
}

bool ChannelWin::ProcessMessageForDelivery(cr::RefPtr<cr::IOBuffer> message) {
  // |output_queue_| takes ownership of |message|.
  OutputElement* element = new OutputElement(message.get());
  output_queue_.push(element);

  // ensure waiting to write
  if (!waiting_connect_) {
    if (!output_state_.is_pending) {
      if (!ProcessOutgoingMessages(NULL, 0))
        return false;
    }
  }
  return true;
}

void ChannelWin::FlushPrelimQueue() {
  CR_DCHECK(connected_);

  // Due to the possibly re-entrant nature of ProcessMessageForDelivery(), it
  // is critical that |prelim_queue_| appears empty.
  std::queue<cr::RefPtr<cr::IOBuffer>> prelim_queue;
  prelim_queue_.swap(prelim_queue);

  while (!prelim_queue.empty()) {
    cr::RefPtr<cr::IOBuffer> m = prelim_queue.front();
    bool success = ProcessMessageForDelivery(m);
    prelim_queue.pop();

    if (!success)
      break;
  }

  // Delete any unprocessed messages.
  while (!prelim_queue.empty()) {
    prelim_queue.pop();
  }
}

// static
bool ChannelWin::IsNamedServerInitialized(
    const std::string& channel_id) {
  if (::WaitNamedPipeW(PipeName(channel_id).c_str(), 1))
    return true;
  // If ERROR_SEM_TIMEOUT occurred, the pipe exists but is handling another
  // connection.
  return GetLastError() == ERROR_SEM_TIMEOUT;
}

ChannelWin::ReadState ChannelWin::ReadData(
    char* buffer,
    int buffer_len,
    int* /* bytes_read */) {
  if (!pipe_.IsValid())
    return READ_FAILED;

  DWORD bytes_read = 0;
  BOOL ok = ::ReadFile(pipe_.Get(), buffer, buffer_len,
                       &bytes_read, &input_state_.context.overlapped);
  if (!ok) {
    DWORD err = GetLastError();
    if (err == ERROR_IO_PENDING) {
      input_state_.is_pending = true;
      return READ_PENDING;
    }
    CR_LOG(Error) << "pipe error: " << err;
    return READ_FAILED;
  }

  // We could return READ_SUCCEEDED here. But the way that this code is
  // structured we instead go back to the message loop. Our completion port
  // will be signalled even in the "synchronously completed" state.
  //
  // This allows us to potentially process some outgoing messages and
  // interleave other work on this thread when we're getting hammered with
  // input messages. Potentially, this could be tuned to be more efficient
  // with some testing.
  input_state_.is_pending = true;
  return READ_PENDING;
}

bool ChannelWin::DidEmptyInputBuffers() {
  // We don't need to do anything here.
  return true;
}

void ChannelWin::OnTranslatedMessage() {
  if (!connected_) {
    connected_ = true;
    FlushPrelimQueue();
  }
}

// static
const cr::string16 ChannelWin::PipeName(const std::string& channel_id) {
  std::string name("\\\\.\\pipe\\cripc.");
  return cr::ASCIIToUTF16(name.append(channel_id));
}

bool ChannelWin::CreatePipe(const ChannelHandle &channel_handle,
                            Mode mode) {
  CR_DCHECK(!pipe_.IsValid());
  cr::string16 pipe_name;
  // If we already have a valid pipe for channel just copy it.
  if (channel_handle.pipe.handle) {
    // TODO(rvargas) crbug.com/415294: ChannelHandle should either go away in
    // favor of two independent entities (name/file), or it should be a move-
    // only type with a cr::File member. In any case, this code should not
    // call DuplicateHandle.
    CR_DCHECK(channel_handle.name.empty());
    pipe_name = L"Not Available";  // Just used for LOG
    // Check that the given pipe confirms to the specified mode.  We can
    // only check for PIPE_TYPE_MESSAGE & PIPE_SERVER_END flags since the
    // other flags (PIPE_TYPE_BYTE, and PIPE_CLIENT_END) are defined as 0.
    DWORD flags = 0;
    ::GetNamedPipeInfo(channel_handle.pipe.handle, &flags, NULL, NULL, NULL);
    CR_DCHECK(!(flags & PIPE_TYPE_MESSAGE));
    if (((mode & MODE_SERVER_FLAG) && !(flags & PIPE_SERVER_END)) ||
        ((mode & MODE_CLIENT_FLAG) && (flags & PIPE_SERVER_END))) {
      CR_LOG(Warning) << "Inconsistent open mode. Mode :" << mode;
      return false;
    }
    HANDLE local_handle;
    if (!DuplicateHandle(GetCurrentProcess(),
                         channel_handle.pipe.handle,
                         GetCurrentProcess(),
                         &local_handle,
                         0,
                         FALSE,
                         DUPLICATE_SAME_ACCESS)) {
      CR_LOG(Warning) << "DuplicateHandle failed. Error :" << GetLastError();
      return false;
    }
    pipe_.Set(local_handle);
  } else if ((mode & MODE_SERVER_FLAG) || (mode & MODE_CLIENT_FLAG)) {
    CR_DCHECK(!channel_handle.pipe.handle);
    const DWORD open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED |
                            FILE_FLAG_FIRST_PIPE_INSTANCE;
    pipe_name = PipeName(channel_handle.name);

    // trying create client first.
    if (mode & MODE_CLIENT_FLAG) {
      pipe_.Set(CreateFileW(pipe_name.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS |
                                FILE_FLAG_OVERLAPPED,
                            NULL));
      waiting_connect_ = false;
    }

    if (!pipe_.IsValid() && (mode & MODE_SERVER_FLAG)) {
      pipe_.Set(CreateNamedPipeW(pipe_name.c_str(),
                                 open_mode,
                                 PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                 1,
                                 Channel::kReadBufferSize,
                                 Channel::kReadBufferSize,
                                 5000,
                                 NULL));
      waiting_connect_ = true;
    }
  } else {
    CR_NOTREACHED();
  }

  if (!pipe_.IsValid()) {
    // If this process is being closed, the pipe may be gone already.
    CR_PLOG(Warning) << "Unable to create pipe \""
                     << cr::WideToUTF8(pipe_name) << "\" in "
                     << (mode & MODE_SERVER_FLAG ? "server" : "client")
                     << " mode";
    return false;
  }

  return true;
}

bool ChannelWin::Connect() {
  CR_DLOG_IF(Warning, thread_check_.get()) << "Connect called more than once";

  if (!thread_check_.get())
    thread_check_.reset(new cr::ThreadChecker());

  if (!pipe_.IsValid()) {
    CR_NOTREACHED();
    return false;
  }

  cr::MessageLoopForIO::current()->RegisterIOHandler(pipe_.Get(), this);

  // Check to see if there is a client connected to our pipe...
  if (waiting_connect_)
    ProcessConnection();

  if (!input_state_.is_pending) {
    // Complete setup asynchronously. By not setting input_state_.is_pending
    // to true, we indicate to OnIOCompleted that this is the special
    // initialization signal.
    cr::MessageLoopForIO::current()->task_runner()->PostTask(
        CR_FROM_HERE,
        cr::BindOnce(&ChannelWin::OnIOCompleted,
                     weak_factory_.GetWeakPtr(),
                     &input_state_.context,
                     0,
                     0));
  }

  if (!waiting_connect_) {
    connected_ = true;
    ProcessOutgoingMessages(NULL, 0);
  }
  return true;
}

bool ChannelWin::ProcessConnection() {
  CR_DCHECK(thread_check_->CalledOnValidThread());

  if (input_state_.is_pending)
    input_state_.is_pending = false;

  // Do we have a client connected to our pipe?
  if (!pipe_.IsValid())
    return false;

  BOOL ok = ::ConnectNamedPipe(pipe_.Get(), &input_state_.context.overlapped);
  DWORD err = GetLastError();
  if (ok) {
    // Uhm, the API documentation says that this function should never
    // return success when used in overlapped mode.
    CR_NOTREACHED();
    return false;
  }

  switch (err) {
  case ERROR_IO_PENDING:
    input_state_.is_pending = true;
    break;
  case ERROR_PIPE_CONNECTED:
    waiting_connect_ = false;
    break;
  case ERROR_NO_DATA:
    // The pipe is being closed.
    return false;
  default:
    CR_NOTREACHED();
    return false;
  }

  return true;
}

bool ChannelWin::ProcessOutgoingMessages(
    cr::MessageLoopForIO::IOContext* context,
    DWORD bytes_written) {
  CR_DCHECK(!waiting_connect_); // Why are we trying to send messages if
                                // there's no connection?
  CR_DCHECK(thread_check_->CalledOnValidThread());

  if (output_state_.is_pending) {
    CR_DCHECK(context);
    output_state_.is_pending = false;
    if (!context || bytes_written == 0) {
      DWORD err = GetLastError();
      CR_LOG(Error) << "pipe error: " << err;
      return false;
    }
    // Message was sent.
    CR_CHECK(!output_queue_.empty());
    OutputElement* element = output_queue_.front();
    output_queue_.pop();
    delete element;
  }

  if (output_queue_.empty())
    return true;

  if (!pipe_.IsValid())
    return false;

  // Write to pipe...
  OutputElement* element = output_queue_.front();
  CR_DCHECK(element->size() <= INT_MAX);
  BOOL ok = WriteFile(pipe_.Get(),
                      element->data(),
                      static_cast<uint32_t>(element->size()),
                      NULL,
                      &output_state_.context.overlapped);
  if (!ok) {
    DWORD write_error = GetLastError();
    if (write_error == ERROR_IO_PENDING) {
      output_state_.is_pending = true;

      const cr::IOBuffer* m = element->get_message();
      if (m) {
        CR_DVLOG(2) << "sent pending message @" << m << " on channel @" << this
                    << " with size " << m->size();
      }
      return true;
    }
    CR_PLOG(Error) << "pipe error:";
    return false;
  }

  const cr::IOBuffer* m = element->get_message();
  if (m) {
    CR_DVLOG(2) << "sent message @" << m << " on channel @" << this
                << " with size " << m->size();
  }
  output_state_.is_pending = true;
  return true;
}

void ChannelWin::OnIOCompleted(
    cr::MessageLoopForIO::IOContext* context,
    DWORD bytes_transfered,
    DWORD error) {
  CR_DCHECK(thread_check_->CalledOnValidThread());
  bool ok = true;
  if (context == &input_state_.context) {
    if (waiting_connect_) {
      if (!ProcessConnection())
        return;
      // We may have some messages queued up to send...
      if (!output_queue_.empty() && !output_state_.is_pending)
        ProcessOutgoingMessages(NULL, 0);
      if (input_state_.is_pending)
        return;
      // else, fall-through and look for incoming messages...
    }

    // We don't support recursion through OnMessageReceived yet!
    CR_DCHECK(!processing_incoming_);
    cr::AutoReset<bool> auto_reset_processing_incoming(
        &processing_incoming_, true);

    // Process the new data.
    if (input_state_.is_pending) {
      // This is the normal case for everything except the initialization step.
      input_state_.is_pending = false;
      if (!bytes_transfered) {
        ok = false;
      } else if (pipe_.IsValid()) {
        ok = (AsyncReadComplete(bytes_transfered) != DISPATCH_ERROR);
      }
    } else {
      CR_DCHECK(!bytes_transfered);
    }

    // Request more data.
    if (ok)
      ok = (ProcessIncomingData() != DISPATCH_ERROR);
  } else {
    CR_DCHECK(context == &output_state_.context);
    CR_CHECK(output_state_.is_pending);
    ok = ProcessOutgoingMessages(context, bytes_transfered);
  }
  if (!ok && pipe_.IsValid()) {
    // We don't want to re-enter Close().
    delegate()->OnChannelError();
    Close();
  }
}

//------------------------------------------------------------------------------
// Channel's methods

// static
std::unique_ptr<Channel> Channel::Create(const ChannelHandle& channel_handle,
                                         Mode mode,
                                         Delegate* delegate) {
  return std::unique_ptr<Channel>(
      new ChannelWin(channel_handle, mode, delegate));
}

// static
bool Channel::IsNamedServerInitialized(const std::string& channel_id) {
  return ChannelWin::IsNamedServerInitialized(channel_id);
}

}  // namespace cripc

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_CORE_CHANNEL_H_
#define MOJO_CORE_CHANNEL_H_

#include <vector>

#include "crbase/containers/span.h"
#include "crbase/memory/ref_counted.h"
#include "crbase_runtime/single_thread_task_runner.h"
#include "crbase_runtime/io_buffer.h"
#include "cripc/connection_params.h"
#include "crbuild/build_config.h"

namespace mojo {
namespace core {

const size_t kChannelMessageAlignment = 8;

constexpr bool IsAlignedForChannelMessage(size_t n) {
  return n % kChannelMessageAlignment == 0;
}

// Channel provides a thread-safe interface to read and write arbitrary
// delimited messages over an underlying I/O channel, optionally transferring
// one or more platform handles in the process.
class CRIPC_EXPORT Channel
    : public cr::RefCountedThreadSafe<Channel> {
 public:
  // Error types which may be reported by a Channel instance to its delegate.
  enum class Error {
    // The remote end of the channel has been closed, either explicitly or
    // because the process which hosted it is gone.
    kDisconnected,

    // For connection-oriented channels (e.g. named pipes), an unexpected error
    // occurred during channel connection.
    kConnectionFailed,

    // Some incoming data failed validation, implying either a buggy or
    // compromised sender.
    kReceivedMalformedData,
  };

  using AlignedBuffer = std::unique_ptr<char, cr::FreeDeleter>;

  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;

  // Delegate methods are called from the I/O task runner with which the Channel
  // was created (see Channel::Create).
  class Delegate {
   public:
    virtual ~Delegate() = default;

    // Notify of a received message. |payload| is not owned and must not be
    // retained; it will be null if |payload_size| is 0.
    virtual void OnChannelMessage(const void* payload,
                                  size_t payload_size) = 0;

    // Notify that an error has occured and the Channel will cease operation.
    virtual void OnChannelError(Error error) = 0;
  };

  // Creates a new Channel around a |platform_handle|, taking ownership of the
  // handle. All I/O on the handle will be performed on |io_task_runner|.
  // Note that ShutDown() MUST be called on the Channel some time before
  // |delegate| is destroyed.
  static cr::RefPtr<Channel> Create(
      Delegate* delegate,
      ConnectionParams connection_params,
      cr::RefPtr<cr::SingleThreadTaskRunner> io_task_runner);

#if defined(MINI_CHROMIUM_OS_POSIX)
  // At this point only ChannelPosix needs InitFeatures.
  static void set_posix_use_writev(bool use_writev);
#endif  // defined(MINI_CHROMIUM_OS_POSIX)

  // Request that the channel be shut down. This should always be called before
  // releasing the last reference to a Channel to ensure that it's cleaned up
  // on its I/O task runner's thread.
  //
  // Delegate methods will no longer be invoked after this call.
  void ShutDown();

  // Begin processing I/O events. Delegate methods must only be invoked after
  // this call.
  virtual void Start() = 0;

  // Stop processing I/O events.
  virtual void ShutDownImpl() = 0;

  // Queues an outgoing message on the Channel. This message will either
  // eventually be written or will fail to write and trigger
  // Delegate::OnChannelError.
  virtual void Write(cr::RefPtr<cr::IOBuffer> message) = 0;

  // Causes the platform handle to leak when this channel is shut down instead
  // of closing it.
  virtual void LeakHandle() = 0;

 protected:
  // Constructor for implementations to call. |delegate| should be passed 
  // from Create(). .
  Channel(Delegate* delegate);
  virtual ~Channel();

  Delegate* delegate() const { return delegate_; }

  // Called by the implementation when it wants somewhere to stick data.
  // |*buffer_capacity| may be set by the caller to indicate the desired buffer
  // size. If 0, a sane default size will be used instead.
  //
  // Returns the address of a buffer which can be written to, and indicates its
  // actual capacity in |*buffer_capacity|.
  //
  // This should only be used with DispatchBufferPolicy::kManaged.
  char* GetReadBuffer(size_t* buffer_capacity);

  // Called by the implementation when new data is available in the read
  // buffer. Returns false to indicate an error. Upon success,
  // |*next_read_size_hint| will be set to a recommended size for the next
  // read done by the implementation. This should only be used with
  // DispatchBufferPolicy::kManaged.
  bool OnReadComplete(size_t bytes_read, size_t* next_read_size_hint);

  // Called by the implementation to deserialize a message stored in |buffer|.
  // If the channel was created with DispatchBufferPolicy::kUnmanaged, the
  // implementation should call this directly. If it was created with kManaged,
  // OnReadComplete() will call this. |*size_hint| will be set to a recommended
  // size for the next read done by the implementation.
  enum class DispatchResult {
    // The message was dispatched and consumed. |size_hint| contains the size
    // of the message.
    kOK,
    // The message could not be deserialized because |buffer| does not contain
    // enough data. |size_hint| contains the amount of data missing.
    kNotEnoughData,
    // The message has associated handles that were not transferred in this
    // message.
    kMissingHandles,
    // An error occurred during processing.
    kError,
  };
  DispatchResult TryDispatchMessage(cr::Span<const char> buffer,
                                    size_t* size_hint);

  // Called by the implementation when something goes horribly wrong. It is NOT
  // OK to call this synchronously from any public interface methods.
  void OnError(Error error);

 private:
  friend class cr::RefCountedThreadSafe<Channel>;

  class ReadBuffer;

  Delegate* delegate_;
  const std::unique_ptr<ReadBuffer> read_buffer_;
};

}  // namespace core
}  // namespace mojo

#endif  // MOJO_CORE_CHANNEL_H_
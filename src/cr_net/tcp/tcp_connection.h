// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CONNECTION_H_
#define MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CONNECTION_H_

#include <memory>
#include <string>
#include <queue>

#include "cr_base/containers/queue.h"
#include "cr_base/memory/ref_counted.h"
#include "cr_base/memory/weak_ptr.h"
#include "cr_event/io_buffer.h"
#include "cr_net/net_export.h"
#include "cr_build/build_config.h"

namespace crnet {

class StreamSocket;
class TCPServer;
class TCPClient;

// A container which has all information of an Tcp connection. It includes
// id, underlying socket, and pending read/write data.
class CRNET_EXPORT TCPConnection {
 public:
#if defined(MINI_CHROMIUM_ARCH_CPU_64_BITS)
  using Id = uint64_t;
#else
  using Id = uint32_t;
#endif

  // IOBuffer for data read.  It's a wrapper around GrowableIOBuffer, with more
  // functions for buffer management.  It moves unconsumed data to the start of
  // buffer.
  class CRNET_EXPORT ReadIOBuffer : public cr::IOBuffer {
   public:
    static const int kInitialBufSize = 1024;
    static const int kMinimumBufSize = 128;
    static const int kCapacityIncreaseFactor = 2;
    static const int kDefaultMaxBufferSize = 1 * 1024 * 1024;  // 1 Mbytes.

    ReadIOBuffer();

    // Capacity.
    int GetCapacity() const;
    void SetCapacity(int capacity);
    // Increases capacity and returns true if capacity is not beyond the limit.
    bool IncreaseCapacity();

    // Returns a span containing bytes that have been written to, and thus are
    // available to be read from.
    cr::Span<const uint8_t> readable_bytes() const;

    // More read data was appended.
    void DidRead(int bytes);
    // Capacity for which more read data can be appended.
    int RemainingCapacity() const;

    // Removes consumed data and moves unconsumed data to the start of buffer.
    void DidConsume(int bytes);

    // Limit of how much internal capacity can increase.
    int max_buffer_size() const { return max_buffer_size_; }
    void set_max_buffer_size(int max_buffer_size) {
      max_buffer_size_ = max_buffer_size;
    }

   private:
    ~ReadIOBuffer() override;

    cr::RefPtr<cr::GrowableIOBuffer> base_;
    int max_buffer_size_;
  };

  // IOBuffer of pending data to write which has a queue of pending data. Each
  // pending data is stored in std::string.  data() is the data of first
  // std::string stored.
  class CRNET_EXPORT QueuedWriteIOBuffer : public cr::IOBuffer {
   public:
    static const int kDefaultMaxBufferSize = 1 * 1024 * 1024;  // 1 Mbytes.

    QueuedWriteIOBuffer(const QueuedWriteIOBuffer&) = delete;
    QueuedWriteIOBuffer& operator=(const QueuedWriteIOBuffer&) = delete;

    QueuedWriteIOBuffer();

    // Whether or not pending data exists.
    bool IsEmpty() const;

    // Appends new pending data and returns true if total size doesn't exceed
    // the limit, |total_size_limit_|.  It would change data() if new data is
    // the first pending data.
    bool Append(const std::string& data);

    // Consumes data and changes data() accordingly.  It cannot be more than
    // GetSizeToWrite().
    void DidConsume(int size);

    // Gets size of data to write this time. It is NOT total data size.
    int GetSizeToWrite() const;

    // Total size of all pending data.
    int total_size() const { return total_size_; }

    // Limit of how much data can be pending.
    int max_buffer_size() const { return max_buffer_size_; }
    void set_max_buffer_size(int max_buffer_size) {
      max_buffer_size_ = max_buffer_size;
    }

   private:
    ~QueuedWriteIOBuffer() override;

    // This needs to indirect since we need pointer stability for the payload
    // chunks, as they may be handed out via cr::IOBuffer::data().
    cr::Queue<std::unique_ptr<std::string>> pending_data_;
    int total_size_;
    int max_buffer_size_;
  };

  TCPConnection(TCPConnection::Id id, std::unique_ptr<StreamSocket> socket);
  ~TCPConnection();

  Id id() const { return id_; }
  StreamSocket* socket() const { return socket_.get(); }

 private:
  friend class crnet::TCPServer;
  friend class crnet::TCPClient;

  ReadIOBuffer* read_buf() const { return read_buf_.get(); }
  QueuedWriteIOBuffer* write_buf() const { return write_buf_.get(); }

 private:
  const Id id_;
  const std::unique_ptr<StreamSocket> socket_;
  const cr::RefPtr<ReadIOBuffer> read_buf_;
  const cr::RefPtr<QueuedWriteIOBuffer> write_buf_;
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_TCP_TCP_CONNECTION_H_
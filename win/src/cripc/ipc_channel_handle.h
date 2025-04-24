// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRIPC_IPC_CHANNEL_HANDLE_H_
#define MINI_CHROMIUM_SRC_CRIPC_IPC_CHANNEL_HANDLE_H_

#include <string>

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_POSIX)
#include "crbase/file_descriptor_posix.h"
#elif defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#endif  // defined (MINI_CHROMIUM_OS_WIN)

// On Windows, any process can create an IPC channel and others can fetch
// it by name.  We pass around the channel names over IPC.
// On Windows the initialization of ChannelHandle with an existing pipe
// handle is provided for convenience.
// NOTE: A ChannelHandle with a pipe handle Will NOT be marshalled over IPC.

// On POSIX, we instead pass around handles to channel endpoints via IPC.
// When it's time to IPC a new channel endpoint around, we send both the
// channel name as well as a cr::FileDescriptor, which is itself a special
// type that knows how to copy a socket endpoint over IPC.
//
// In sum, this data structure can be used to pass channel information by name
// in both Windows and Posix. When passing a handle to a channel over IPC,
// use this data structure only for POSIX.

namespace cripc {

struct ChannelHandle {
  // Note that serialization for this object is defined in the ParamTraits
  // template specialization in ipc_message_utils.h.
  ChannelHandle() {}
  // The name that is passed in should be an absolute path for Posix.
  // Otherwise there may be a problem in IPC communication between
  // processes with different working directories.
  ChannelHandle(const std::string& n) : name(n) {}
  ChannelHandle(const char* n) : name(n) {}
#if defined(MINI_CHROMIUM_OS_WIN)
  explicit ChannelHandle(HANDLE h) : pipe(h) {}
#elif defined(MINI_CHROMIUM_OS_POSIX)
  ChannelHandle(const std::string& n, const cr::FileDescriptor& s)
      : name(n), socket(s) {}
#endif  // defined(MINI_CHROMIUM_OS_POSIX)

  std::string name;
#if defined(MINI_CHROMIUM_OS_POSIX)
  cr::FileDescriptor socket;
#elif defined(MINI_CHROMIUM_OS_WIN)
  // A simple container to automatically initialize pipe handle
  struct PipeHandle {
    PipeHandle() : handle(NULL) {}
    PipeHandle(HANDLE h) : handle(h) {}
    HANDLE handle;
  };
  PipeHandle pipe;
#endif  // defined (MINI_CHROMIUM_OS_WIN)
};

}  // namespace cripc

#endif  // MINI_CHROMIUM_SRC_CRIPC_IPC_CHANNEL_HANDLE_H_
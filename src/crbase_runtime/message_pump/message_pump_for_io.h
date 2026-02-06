// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_MESSAGE_PUMP_MESSAGE_PUMP_FOR_IO_H_
#define MINI_CHROMIUM_SRC_CRBASE_MESSAGE_PUMP_MESSAGE_PUMP_FOR_IO_H_

// This header is a forwarding header to coalesce the various platform specific
// types representing MessagePumpForIO.

#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase_runtime/message_pump/message_pump_win.h"
#elif defined(MINI_CHROMIUM_OS_LINUX)
#include "crbase_runtime/message_pump/message_pump_epoll.h"
#endif

namespace cr {

#if defined(MINI_CHROMIUM_OS_WIN)
// Windows defines it as-is.
using MessagePumpForIO = MessagePumpForIO;
#elif defined(MINI_CHROMIUM_OS_LINUX)
using MessagePumpForIO = MessagePumpEpoll;
#else
#error Platform does not define MessagePumpForIO
#endif

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MESSAGE_PUMP_MESSAGE_PUMP_FOR_IO_H_

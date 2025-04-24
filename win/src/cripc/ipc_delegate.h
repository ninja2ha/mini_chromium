// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRIPC_IPC_LISTENER_H_
#define MINI_CHROMIUM_SRC_CRIPC_IPC_LISTENER_H_

#include <stdint.h>

#include "cripc/ipc_export.h"

namespace cripc {

// Implemented by consumers of a Channel to receive messages.
class CRIPC_EXPORT Delegate {
 public:
  // Called when the channel created.
  virtual void OnChannelCreated() {};

  // Called when input data is received.  Returns numbers of bytes handled.
  // The value is less-than 0 while error occured. 
  virtual int OnChannelTranslateData(const char* data_start, 
                                     const char* data_end) { 
    return  static_cast<int>(data_end - data_end); 
  }

  // Called when the channel created.
  virtual void OnChannelClosed() {};

  // Called when an error is detected that causes the channel to close.
  // This method is not called when a channel is closed normally.
  virtual void OnChannelError() {}

 protected:
  virtual ~Delegate() {}
};

}  // namespace cripc

#endif  // MINI_CHROMIUM_SRC_CRIPC_IPC_LISTENER_H_
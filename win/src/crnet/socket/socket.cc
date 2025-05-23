// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crnet/socket/socket.h"

#include "crnet/base/net_errors.h"

namespace crnet {

int Socket::ReadIfReady(cr::IOBuffer* buf,
                        int buf_len,
                        CompletionOnceCallback callback) {
  return ERR_READ_IF_READY_NOT_IMPLEMENTED;
}

int Socket::CancelReadIfReady() {
  return ERR_READ_IF_READY_NOT_IMPLEMENTED;
}

}  // namespace crnet
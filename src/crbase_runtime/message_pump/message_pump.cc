// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 71.0.3578.141

#include "crbase_runtime/message_pump/message_pump.h"

namespace cr {

MessagePump::MessagePump() = default;

MessagePump::~MessagePump() = default;

void MessagePump::SetTimerSlack(TimerSlack) {
}

}  // namespace cr
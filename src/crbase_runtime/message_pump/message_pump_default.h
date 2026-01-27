// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H_

#include "crbase/base_export.h"
#include "crbase/synchronization/waitable_event.h"
#include "crbase/time/time.h"
#include "crbase_runtime/message_pump/message_pump.h"
#include "crbuild/build_config.h"

namespace cr {

class CRBASE_EXPORT MessagePumpDefault : public MessagePump {
 public:
  MessagePumpDefault(const MessagePumpDefault&) = delete;
  MessagePumpDefault& operator=(const MessagePumpDefault&) = delete;

  MessagePumpDefault();
  ~MessagePumpDefault() override;

  // MessagePump methods:
  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

 private:
  // This flag is set to false when Run should return.
  bool keep_running_;

  // Used to sleep until there is more work to do.
  WaitableEvent event_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H_

// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/message_pump/message_pump.h"

#include "crbase/logging/logging.h"
#include "crbase_runtime/message_pump/message_pump_default.h"
#include "crbase_runtime/message_pump/message_pump_for_io.h"
#include "crbase_runtime/message_pump/message_pump_for_ui.h"

namespace cr {

namespace {

MessagePump::MessagePumpFactory* message_pump_for_ui_factory_ = nullptr;

}  // namespace

MessagePump::MessagePump() = default;

MessagePump::~MessagePump() = default;

void MessagePump::SetTimerSlack(TimerSlack) {
}

// static
void MessagePump::OverrideMessagePumpForUIFactory(MessagePumpFactory* factory) {
  CR_DCHECK(!message_pump_for_ui_factory_);
  message_pump_for_ui_factory_ = factory;
}

// static
bool MessagePump::IsMessagePumpForUIFactoryOveridden() {
  return message_pump_for_ui_factory_ != nullptr;
}

// static
std::unique_ptr<MessagePump> MessagePump::Create(MessagePumpType type) {
  switch (type) {
    case MessagePumpType::UI:
      if (message_pump_for_ui_factory_)
        return message_pump_for_ui_factory_();
      return std::make_unique<MessagePumpForUI>();

    case MessagePumpType::IO:
      return std::make_unique<MessagePumpForIO>();

#if defined(MINI_CHROMIUM_OS_WIN)
    case MessagePumpType::UI_WITH_WM_QUIT_SUPPORT: {
      auto pump = std::make_unique<MessagePumpForUI>();
      pump->EnableWmQuit();
      return pump;
    }
#endif  // defined(OS_WIN)

    case MessagePumpType::CUSTOM:
      CR_NOTREACHED();
      return nullptr;

    case MessagePumpType::DEFAULT:
      return std::make_unique<MessagePumpDefault>();
  }

  CR_NOTREACHED();
  return nullptr;
}

}  // namespace cr

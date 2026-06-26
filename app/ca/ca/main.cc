// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/logging/logging.h"
#include "cr_base/at_exit.h"
#include "cr_base/net/ip_address.h"

#include "cr_event/task/single_thread_task_executor.h"
#include "cr_event/task/sequenced_task_runner_handle.h"
#include "cr_event/timer/timer.h"
#include "cr_event/run_loop.h"

namespace {

class ScopedInitLogging {
 public:
  ScopedInitLogging() {
    auto& config = CR_DEFAULT_LOGGING_CONFIG;
    config.logging_dest = cr::logging::LOG_TO_STDERR;
    config.verbose_lowest_level = 0;

    cr::logging::InitializeConfig(config);
  }
  ~ScopedInitLogging() {
    cr::logging::UninitializeConfig(CR_DEFAULT_LOGGING_CONFIG);
  }
};

}  // namesapce

// -----------------------------------------------------------------------------

int main(int argc, char* argv) {
  ScopedInitLogging logging;

  cr::AtExitManager at_exit_manager;
  cr::SingleThreadTaskExecutor message_loop(cr::MessagePumpType::UI);

  cr::SequencedTaskRunnerHandle::Get()->PostTask(CR_FROM_HERE, cr::BindOnce([]{
    CR_LOG(Info) << "init..";
  }));

  cr::net::IPAddress ip_address;
  ip_address.AssignFromIPLiteral("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
  CR_LOG(Info) << "ip_address=" << ip_address.ToString();

  cr::RunLoop run_loop;
  run_loop.Run(CR_FROM_HERE);
  return 0;
}
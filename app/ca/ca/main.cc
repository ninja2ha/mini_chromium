// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/logging/logging.h"
#include "cr_base/at_exit.h"
#include "cr_base/json/json_writer.h"
#include "cr_base/json/json_reader.h"
#include "cr_base/values.h"

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
  cr::RunLoop run_loop;

  auto task_runner = message_loop.task_runner();

  // task
  task_runner->PostTask(CR_FROM_HERE, cr::BindOnce([]{
    CR_LOG(Info) << "init client.";
  }));

  task_runner->PostDelayedTask(CR_FROM_HERE, cr::BindOnce([] {
    CR_LOG(Info) << "run a delayed task.";
  }), cr::TimeDelta::FromSeconds(1));

  // timer
  std::string output;
  cr::DictionaryValue dict_value;
  dict_value.SetStringKey("a", "123456");
  dict_value.SetIntKey("b", 1);
  cr::JSONWriter::Write(dict_value, &output);
  CR_LOG(Info) << output;

  cr::Optional<cr::Value> v = cr::JSONReader::Read(output);
  const std::string* a = v->FindStringKey("a");
  cr::Optional<int> b = v->FindIntKey("a");
  CR_LOG(Info) << *a << ", " << b.value_or(0);

  run_loop.Run(CR_FROM_HERE);
  return 0;
}
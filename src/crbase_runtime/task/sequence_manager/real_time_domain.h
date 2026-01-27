// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_REAL_TIME_DOMAIN_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_REAL_TIME_DOMAIN_H_

#include "crbase/base_export.h"
#include "crbase_runtime/task/sequence_manager/time_domain.h"

namespace cr {
namespace sequence_manager {
namespace internal {

class CRBASE_EXPORT RealTimeDomain : public TimeDomain {
 public:
  RealTimeDomain() = default;
  RealTimeDomain(const RealTimeDomain&) = delete;
  RealTimeDomain& operator=(const RealTimeDomain&) = delete;
  ~RealTimeDomain() override = default;

  // TimeDomain implementation:
  LazyNow CreateLazyNow() const override;
  TimeTicks Now() const override;
  Optional<TimeDelta> DelayTillNextTask(LazyNow* lazy_now) override;
  bool MaybeFastForwardToNextTask(bool quit_when_idle_requested) override;

 protected:
  void OnRegisterWithSequenceManager(
      SequenceManagerImpl* sequence_manager) override;
  const char* GetName() const override;

 private:
  const TickClock* tick_clock_ = nullptr;
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_REAL_TIME_DOMAIN_H_

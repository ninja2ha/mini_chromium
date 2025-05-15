// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/animation/animation_runner.h"

#include <utility>

#include "crbase/timer/timer.h"

namespace {

// A default AnimationRunner based on cr::Timer.
// TODO(https://crbug.com/953585): Remove this altogether.
class DefaultAnimationRunner : public crui::gfx::AnimationRunner {
 public:
  DefaultAnimationRunner() = default;
  ~DefaultAnimationRunner() override = default;

  // gfx::AnimationRunner:
  void Stop() override;

 protected:
  // gfx::AnimationRunner:
  void OnStart(cr::TimeDelta min_interval, cr::TimeDelta elapsed) override;

 private:
  void OnTimerTick();

  cr::OneShotTimer timer_;
  cr::TimeDelta min_interval_;
};

void DefaultAnimationRunner::Stop() {
  timer_.Stop();
}

void DefaultAnimationRunner::OnStart(cr::TimeDelta min_interval,
                                     cr::TimeDelta elapsed) {
  min_interval_ = min_interval;
  timer_.Start(CR_FROM_HERE, min_interval - elapsed, this,
               &DefaultAnimationRunner::OnTimerTick);
}

void DefaultAnimationRunner::OnTimerTick() {
  // This is effectively a RepeatingTimer.  It's possible to use a true
  // RepeatingTimer for this, but since OnStart() may need to use a OneShotTimer
  // anyway (when |elapsed| is nonzero), it's just more complicated.
  timer_.Start(CR_FROM_HERE, min_interval_, this,
               &DefaultAnimationRunner::OnTimerTick);
  // Call Step() after timer_.Start() in case Step() calls Stop().
  Step(cr::TimeTicks::Now());
}

}  // namespace

namespace crui {
namespace gfx {

// static
std::unique_ptr<AnimationRunner>
AnimationRunner::CreateDefaultAnimationRunner() {
  return std::make_unique<DefaultAnimationRunner>();
}

AnimationRunner::~AnimationRunner() = default;

void AnimationRunner::Start(
    cr::TimeDelta min_interval,
    cr::TimeDelta elapsed,
    cr::RepeatingCallback<void(cr::TimeTicks)> step) {
  step_ = std::move(step);
  OnStart(min_interval, elapsed);
}

AnimationRunner::AnimationRunner() = default;

void AnimationRunner::Step(cr::TimeTicks tick) {
  step_.Run(tick);
}

void AnimationRunner::SetAnimationTimeForTesting(cr::TimeTicks time) {
  step_.Run(time);
}

}  // namespace gfx
}  // namespace crui

// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_ANIMATION_ANIMATION_RUNNER_H_
#define UI_GFX_ANIMATION_ANIMATION_RUNNER_H_

#include <memory>

#include "crbase/functional/callback.h"
#include "crbase/time/time.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace gfx {

// Interface for custom animation runner. CompositorAnimationRunner can control
// animation tick with this.
class CRUI_EXPORT AnimationRunner {
 public:
  // Creates a default AnimationRunner based on base::Timer. Ideally,
  // we should prefer the compositor-based animation runner to this.
  // TODO(https://crbug.com/953585): Remove this altogether.
  static std::unique_ptr<AnimationRunner> CreateDefaultAnimationRunner();

  AnimationRunner(const AnimationRunner&) = delete;
  AnimationRunner& operator=(const AnimationRunner&) = delete;
  virtual ~AnimationRunner();

  // Sets the provided |step| callback, then calls OnStart() with the provided
  // |min_interval| and |elapsed| time to allow the subclass to actually begin
  // animating. Subclasses are expected to call Step() periodically to drive the
  // animation.
  void Start(cr::TimeDelta min_interval,
             cr::TimeDelta elapsed,
             cr::RepeatingCallback<void(cr::TimeTicks)> step);

  // Called when subclasses don't need to call Step() anymore.
  virtual void Stop() = 0;

  bool step_is_null_for_testing() const { return step_.is_null(); }

 protected:
  AnimationRunner();

  // Called when subclasses should start calling Step() periodically to
  // drive the animation.
  virtual void OnStart(cr::TimeDelta min_interval,
                       cr::TimeDelta elapsed) = 0;

  // Advances the animation based on |tick|.
  void Step(cr::TimeTicks tick);

 private:
  friend class AnimationContainerTestApi;

  // Advances the animation manually for testing.
  void SetAnimationTimeForTesting(cr::TimeTicks time);

  cr::RepeatingCallback<void(cr::TimeTicks)> step_;
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_ANIMATION_ANIMATION_RUNNER_H_

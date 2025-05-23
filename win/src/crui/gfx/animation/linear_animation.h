// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_ANIMATION_LINEAR_ANIMATION_H_
#define UI_GFX_ANIMATION_LINEAR_ANIMATION_H_

#include "crbase/time/time.h"
#include "crui/gfx/animation/animation.h"

namespace crui {
namespace gfx {

class AnimationDelegate;

// Linear time bounded animation. As the animation progresses AnimateToState is
// invoked.
class CRUI_EXPORT LinearAnimation : public Animation {
 public:
  // Default frame rate (hz).
  static const int kDefaultFrameRate;

  LinearAnimation(const LinearAnimation&) = delete;
  LinearAnimation& operator=(const LinearAnimation&) = delete;

  // Initializes everything except the duration.
  //
  // Caller must make sure to call SetDuration() if they use this
  // constructor; it is preferable to use the full one, but sometimes
  // duration can change between calls to Start() and we need to
  // expose this interface.
  explicit LinearAnimation(AnimationDelegate* delegate,
                           int frame_rate = kDefaultFrameRate);

  // Initializes all fields.
  LinearAnimation(cr::TimeDelta duration,
                  int frame_rate,
                  AnimationDelegate* delegate);

  // Gets the value for the current state, according to the animation curve in
  // use. This class provides only for a linear relationship, however subclasses
  // can override this to provide others.
  double GetCurrentValue() const override;

  // Change the current state of the animation to |new_value|.
  void SetCurrentValue(double new_value);

  // Skip to the end of the current animation.
  void End();

  // Changes the length of the animation. This resets the current
  // state of the animation to the beginning. This value will be multiplied by
  // the currently set scale factor.
  void SetDuration(cr::TimeDelta duration);

  // Sets the duration scale factor. This scale factor will be applied to all
  // animation durations globally. This value must be >= 0. The default value
  // is 1.0.
  static void SetDurationScale(double scale_factor);

 protected:
  // Called when the animation progresses. Subclasses override this to
  // efficiently update their state.
  virtual void AnimateToState(double state) {}

  // Invoked by the AnimationContainer when the animation is running to advance
  // the animation. Use |time_now| rather than Time::Now to avoid multiple
  // animations running at the same time diverging.
  void Step(cr::TimeTicks time_now) override;

  // Overriden to initialize state.
  void AnimationStarted() override;

  // Overriden to advance to the end (if End was invoked).
  void AnimationStopped() override;

  // Overriden to return true if state is not 1.
  bool ShouldSendCanceledFromStop() override;

  cr::TimeDelta duration() const { return duration_; }

 private:
  cr::TimeDelta duration_;

  // Current state, on a scale from 0.0 to 1.0.
  double state_;

  // If true, we're in end. This is used to determine if the animation should
  // be advanced to the end from AnimationStopped.
  bool in_end_;
};

}  // namespace gfx
}  // namespace crui

#endif  // APP_LINEAR_ANIMATION_H_

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_ANIMATION_MULTI_ANIMATION_H_
#define UI_GFX_ANIMATION_MULTI_ANIMATION_H_

#include <stddef.h>

#include <vector>

#include "crbase/time/time.h"
#include "crui/gfx/animation/animation.h"
#include "crui/gfx/animation/tween.h"

namespace crui {
namespace gfx {

// MultiAnimation is an animation that consists of a number of sub animations.
// To create a MultiAnimation pass in the parts, invoke Start() and the delegate
// is notified as the animation progresses. By default MultiAnimation runs until
// Stop is invoked, see |set_continuous()| for details.
class CRUI_EXPORT MultiAnimation : public Animation {
 public:
  // Defines part of the animation. Each part consists of the following:
  //
  // part_length: the length of time the part runs.
  // part_start: the amount of time to offset this part by when calculating the
  // initial percentage.
  // total_length: the total length used to calculate the percentange completed.
  //
  // In most cases |part_start| is empty and |total_length| = |part_length|. But
  // you can adjust the start/total for different effects. For example, to run a
  // part for 200ms with a % between .25 and .75 use the following three values:
  // part_length = 200, part_start = 100, total_length = 400.
  struct Part {
    Part() : Part(cr::TimeDelta(), Tween::ZERO) {}
    Part(cr::TimeDelta part_length, Tween::Type type)
        : Part(part_length, cr::TimeDelta(), part_length, type) {}
    Part(cr::TimeDelta part_length,
         cr::TimeDelta part_start,
         cr::TimeDelta total_length,
         Tween::Type type)
        : part_length(part_length),
          part_start(part_start),
          total_length(total_length),
          type(type) {}

    cr::TimeDelta part_length;
    cr::TimeDelta part_start;
    cr::TimeDelta total_length;
    Tween::Type type;
  };
  using Parts = std::vector<Part>;

  static constexpr auto GetDefaultTimerInterval() {
    return cr::TimeDelta::FromMilliseconds(20);
  }

  MultiAnimation(const MultiAnimation&) = delete;
  MultiAnimation& operator=(const MultiAnimation&) = delete;

  MultiAnimation(const Parts& parts, cr::TimeDelta timer_interval);
  ~MultiAnimation() override;

  // Sets whether the animation continues after it reaches the end. If true, the
  // animation runs until explicitly stopped. The default is true.
  void set_continuous(bool continuous) { continuous_ = continuous; }

  // Returns the current value. The current value for a MultiAnimation is
  // determined from the tween type of the current part.
  double GetCurrentValue() const override;

  // Returns the index of the current part.
  size_t current_part_index() const { return current_part_index_; }

 protected:
  // Animation overrides.
  void Step(cr::TimeTicks time_now) override;
  void SetStartTime(cr::TimeTicks start_time) override;

 private:
  // Returns the part containing the specified time. |time| is reset to be
  // relative to the part containing the time and |part_index| the index of the
  // part.
  const Part& GetPart(cr::TimeDelta* time, size_t* part_index);

  // The parts that make up the animation.
  const Parts parts_;

  // Total time of all the parts.
  const cr::TimeDelta cycle_time_;

  // Current value for the animation.
  double current_value_;

  // Index of the current part.
  size_t current_part_index_;

  // See description above setter.
  bool continuous_;
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_ANIMATION_MULTI_ANIMATION_H_

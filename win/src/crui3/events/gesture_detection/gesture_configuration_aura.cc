// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/gesture_detection/gesture_configuration.h"

#include "crbase/memory/singleton.h"
#include "crui/events/event_switches.h"

namespace crui {
namespace {

constexpr bool kDoubleTapAuraSupport = false;

class GestureConfigurationAura : public GestureConfiguration {
 public:
  GestureConfigurationAura(const GestureConfigurationAura&) = delete;
  GestureConfigurationAura& operator=(const GestureConfigurationAura&) = delete;

  ~GestureConfigurationAura() override {
  }

  static GestureConfigurationAura* GetInstance() {
    return cr::Singleton<GestureConfigurationAura>::get();
  }

 private:
  GestureConfigurationAura() : GestureConfiguration() {
    set_double_tap_enabled(kDoubleTapAuraSupport);
    set_double_tap_timeout_in_ms(semi_long_press_time_in_ms());
    set_gesture_begin_end_types_enabled(true);
    set_min_gesture_bounds_length(default_radius());
    set_min_pinch_update_span_delta(5/*
        base::CommandLine::ForCurrentProcess()->HasSwitch(
            switches::kCompensateForUnstablePinchZoom)
            ? 5
            : 0*/);
    set_velocity_tracker_strategy(VelocityTracker::Strategy::LSQ2_RESTRICTED);
    set_span_slop(max_touch_move_in_pixels_for_click() * 2);
    set_swipe_enabled(true);
    set_two_finger_tap_enabled(true);
    set_fling_touchpad_tap_suppression_enabled(true);
    set_fling_touchscreen_tap_suppression_enabled(true);
  }

  friend struct cr::DefaultSingletonTraits<GestureConfigurationAura>;
};

}  // namespace

// Create a GestureConfigurationAura singleton instance when using aura.
GestureConfiguration* GestureConfiguration::GetPlatformSpecificInstance() {
  return GestureConfigurationAura::GetInstance();
}

}  // namespace crui

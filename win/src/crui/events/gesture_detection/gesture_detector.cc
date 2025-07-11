// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/gesture_detection/gesture_detector.h"

#include <stddef.h>

#include <algorithm>

#include "crbase/numerics/ranges.h"
#include "crbase/timer/timer.h"
#include "crui/events/gesture_detection/gesture_listeners.h"
#include "crui/events/gesture_detection/motion_event.h"
#include "crui/gfx/geometry/angle_conversions.h"

namespace crui {
namespace {

// Minimum distance a scroll must have traveled from the last scroll/focal point
// to trigger an |OnScroll| callback.
const float kScrollEpsilon = .1f;

// Constants used by TimeoutGestureHandler.
enum TimeoutEvent {
  SHOW_PRESS = 0,
  LONG_PRESS,
  TAP,
  TIMEOUT_EVENT_COUNT
};

}  // namespace

// Note: These constants were taken directly from the default (unscaled)
// versions found in Android's ViewConfiguration. Do not change these default
// values without explicitly consulting an OWNER.
GestureDetector::Config::Config()
    : longpress_timeout(cr::TimeDelta::FromMilliseconds(500)),
      showpress_timeout(cr::TimeDelta::FromMilliseconds(180)),
      double_tap_timeout(cr::TimeDelta::FromMilliseconds(300)),
      double_tap_min_time(cr::TimeDelta::FromMilliseconds(40)),
      touch_slop(8),
      double_tap_slop(100),
      minimum_fling_velocity(50),
      maximum_fling_velocity(8000),
      swipe_enabled(false),
      minimum_swipe_velocity(20),
      maximum_swipe_deviation_angle(20.f),
      two_finger_tap_enabled(false),
      two_finger_tap_max_separation(300),
      two_finger_tap_timeout(cr::TimeDelta::FromMilliseconds(700)),
      single_tap_repeat_interval(1),
      velocity_tracker_strategy(VelocityTracker::Strategy::STRATEGY_DEFAULT) {
}

GestureDetector::Config::Config(const Config& other) = default;

GestureDetector::Config::~Config() {}

class GestureDetector::TimeoutGestureHandler {
 public:
  TimeoutGestureHandler(const Config& config, GestureDetector* gesture_detector)
      : gesture_detector_(gesture_detector) {
    CR_DCHECK(config.showpress_timeout <= config.longpress_timeout);

    timeout_callbacks_[SHOW_PRESS] = &GestureDetector::OnShowPressTimeout;
    timeout_delays_[SHOW_PRESS] = config.showpress_timeout;

    timeout_callbacks_[LONG_PRESS] = &GestureDetector::OnLongPressTimeout;
    timeout_delays_[LONG_PRESS] =
        config.longpress_timeout + config.showpress_timeout;

    timeout_callbacks_[TAP] = &GestureDetector::OnTapTimeout;
    timeout_delays_[TAP] = config.double_tap_timeout;
  }

  ~TimeoutGestureHandler() {
    Stop();
  }

  void StartTimeout(TimeoutEvent event) {
    timeout_timers_[event].Start(CR_FROM_HERE,
                                 timeout_delays_[event],
                                 gesture_detector_,
                                 timeout_callbacks_[event]);
  }

  void StopTimeout(TimeoutEvent event) { timeout_timers_[event].Stop(); }

  void Stop() {
    for (size_t i = SHOW_PRESS; i < TIMEOUT_EVENT_COUNT; ++i)
      timeout_timers_[i].Stop();
  }

  bool HasTimeout(TimeoutEvent event) const {
    return timeout_timers_[event].IsRunning();
  }

 private:
  typedef void (GestureDetector::*ReceiverMethod)();

  GestureDetector* const gesture_detector_;
  cr::OneShotTimer timeout_timers_[TIMEOUT_EVENT_COUNT];
  ReceiverMethod timeout_callbacks_[TIMEOUT_EVENT_COUNT];
  cr::TimeDelta timeout_delays_[TIMEOUT_EVENT_COUNT];
};

GestureDetector::GestureDetector(
    const Config& config,
    GestureListener* listener,
    DoubleTapListener* optional_double_tap_listener)
    : timeout_handler_(new TimeoutGestureHandler(config, this)),
      listener_(listener),
      double_tap_listener_(optional_double_tap_listener),
      touch_slop_square_(0),
      double_tap_touch_slop_square_(0),
      double_tap_slop_square_(0),
      two_finger_tap_distance_square_(0),
      min_fling_velocity_(1),
      max_fling_velocity_(1),
      min_swipe_velocity_(0),
      min_swipe_direction_component_ratio_(0),
      still_down_(false),
      defer_confirm_single_tap_(false),
      all_pointers_within_slop_regions_(false),
      always_in_bigger_tap_region_(false),
      two_finger_tap_allowed_for_gesture_(false),
      is_double_tapping_(false),
      is_down_candidate_for_repeated_single_tap_(false),
      maximum_pointer_count_(0),
      current_single_tap_repeat_count_(0),
      single_tap_repeat_interval_(1),
      last_focus_x_(0),
      last_focus_y_(0),
      down_focus_x_(0),
      down_focus_y_(0),
      longpress_enabled_(true),
      showpress_enabled_(true),
      swipe_enabled_(false),
      two_finger_tap_enabled_(false),
      velocity_tracker_(config.velocity_tracker_strategy) {
  CR_DCHECK(listener_);
  Init(config);
}

GestureDetector::~GestureDetector() {}

bool GestureDetector::OnTouchEvent(const MotionEvent& ev,
                                   bool should_process_double_tap) {
  const MotionEvent::Action action = ev.GetAction();

  velocity_tracker_.AddMovement(ev);

  const bool pointer_up = action == MotionEvent::Action::POINTER_UP;
  const int skip_index = pointer_up ? ev.GetActionIndex() : -1;

  // Determine focal point.
  float sum_x = 0, sum_y = 0;
  const int count = static_cast<int>(ev.GetPointerCount());
  for (int i = 0; i < count; i++) {
    if (skip_index == i)
      continue;
    sum_x += ev.GetX(i);
    sum_y += ev.GetY(i);
  }
  const int div = pointer_up ? count - 1 : count;
  const float focus_x = sum_x / div;
  const float focus_y = sum_y / div;

  bool handled = false;

  switch (action) {
    case MotionEvent::Action::NONE:
    case MotionEvent::Action::HOVER_ENTER:
    case MotionEvent::Action::HOVER_EXIT:
    case MotionEvent::Action::HOVER_MOVE:
    case MotionEvent::Action::BUTTON_PRESS:
    case MotionEvent::Action::BUTTON_RELEASE:
      CR_NOTREACHED();
      return handled;

    case MotionEvent::Action::POINTER_DOWN: {
      down_focus_x_ = last_focus_x_ = focus_x;
      down_focus_y_ = last_focus_y_ = focus_y;
      // Cancel long press and taps.
      CancelTaps();
      maximum_pointer_count_ = std::max(maximum_pointer_count_,
                                        static_cast<int>(ev.GetPointerCount()));

      // Even when two_finger_tap_allowed_for_gesture_ is false,
      // second pointer down information must be stored to check
      // the slop region in multi-finger scrolls.
      if (ev.GetPointerCount() == 2)
        secondary_pointer_down_event_ = ev.Clone();

      if (!two_finger_tap_allowed_for_gesture_)
        break;

      const int action_index = ev.GetActionIndex();
      const float dx = ev.GetX(action_index) - current_down_event_->GetX();
      const float dy = ev.GetY(action_index) - current_down_event_->GetY();

      if (maximum_pointer_count_ > 2 ||
          dx * dx + dy * dy >= two_finger_tap_distance_square_)
        two_finger_tap_allowed_for_gesture_ = false;
    } break;

    case MotionEvent::Action::POINTER_UP: {
      down_focus_x_ = last_focus_x_ = focus_x;
      down_focus_y_ = last_focus_y_ = focus_y;

      // Check the dot product of current velocities.
      // If the pointer that left was opposing another velocity vector, clear.
      velocity_tracker_.ComputeCurrentVelocity(1000, max_fling_velocity_);
      const int up_index = ev.GetActionIndex();
      const int id1 = ev.GetPointerId(up_index);
      const float vx1 = velocity_tracker_.GetXVelocity(id1);
      const float vy1 = velocity_tracker_.GetYVelocity(id1);
      float vx_total = vx1;
      float vy_total = vy1;
      for (int i = 0; i < count; i++) {
        if (i == up_index)
          continue;

        const int id2 = ev.GetPointerId(i);
        const float vx2 = velocity_tracker_.GetXVelocity(id2);
        const float vy2 = velocity_tracker_.GetYVelocity(id2);
        const float dot = vx1 * vx2 + vy1 * vy2;
        if (dot < 0) {
          vx_total = 0;
          vy_total = 0;
          velocity_tracker_.Clear();
          break;
        }
        vx_total += vx2;
        vy_total += vy2;
      }

      handled = HandleSwipeIfNeeded(ev, vx_total / count, vy_total / count);

      if (two_finger_tap_allowed_for_gesture_ && ev.GetPointerCount() == 2 &&
          secondary_pointer_down_event_ &&
          (ev.GetEventTime() - secondary_pointer_down_event_->GetEventTime() <=
           two_finger_tap_timeout_)) {
        handled = listener_->OnTwoFingerTap(*current_down_event_, ev);
      }
      two_finger_tap_allowed_for_gesture_ = false;
    } break;

    case MotionEvent::Action::DOWN: {
      bool is_repeated_tap =
          current_down_event_ && previous_up_event_ &&
          IsRepeatedTap(*current_down_event_, *previous_up_event_, ev,
                        should_process_double_tap);
      if (double_tap_listener_ && should_process_double_tap) {
        is_down_candidate_for_repeated_single_tap_ = false;
        bool had_tap_message = timeout_handler_->HasTimeout(TAP);
        if (had_tap_message)
          timeout_handler_->StopTimeout(TAP);
        if (is_repeated_tap && had_tap_message) {
          // This is a second tap.
          is_double_tapping_ = true;
          // Give a callback with the first tap of the double-tap.
          handled |= double_tap_listener_->OnDoubleTap(*current_down_event_);
          // Give a callback with down event of the double-tap.
          handled |= double_tap_listener_->OnDoubleTapEvent(ev);
        } else {
          // This is a first tap.
          CR_DCHECK(double_tap_timeout_ > cr::TimeDelta());
          timeout_handler_->StartTimeout(TAP);
        }
      } else {
        is_down_candidate_for_repeated_single_tap_ = is_repeated_tap;
      }

      down_focus_x_ = last_focus_x_ = focus_x;
      down_focus_y_ = last_focus_y_ = focus_y;
      current_down_event_ = ev.Clone();

      secondary_pointer_down_event_.reset();
      all_pointers_within_slop_regions_ = true;
      always_in_bigger_tap_region_ = true;
      still_down_ = true;
      defer_confirm_single_tap_ = false;
      two_finger_tap_allowed_for_gesture_ = two_finger_tap_enabled_;
      maximum_pointer_count_ = 1;

      // Always start the SHOW_PRESS timer before the LONG_PRESS timer to
      // ensure proper timeout ordering.
      if (showpress_enabled_)
        timeout_handler_->StartTimeout(SHOW_PRESS);
      if (longpress_enabled_)
        timeout_handler_->StartTimeout(LONG_PRESS);
      handled |= listener_->OnDown(ev);
    } break;

    case MotionEvent::Action::MOVE: {
      const float scroll_x = last_focus_x_ - focus_x;
      const float scroll_y = last_focus_y_ - focus_y;
      if (is_double_tapping_) {
        // Give the move events of the double-tap.
        CR_DCHECK(double_tap_listener_);
        handled |= double_tap_listener_->OnDoubleTapEvent(ev);
      } else if (all_pointers_within_slop_regions_) {
        if (!IsWithinTouchSlop(ev)) {
          handled = listener_->OnScroll(
              *current_down_event_, ev,
              (maximum_pointer_count_ > 1 && secondary_pointer_down_event_)
                  ? *secondary_pointer_down_event_
                  : ev,
              scroll_x, scroll_y);
          last_focus_x_ = focus_x;
          last_focus_y_ = focus_y;
          all_pointers_within_slop_regions_ = false;
          timeout_handler_->Stop();
        }

        const float delta_x = focus_x - down_focus_x_;
        const float delta_y = focus_y - down_focus_y_;
        const float distance_square = delta_x * delta_x + delta_y * delta_y;
        if (distance_square > double_tap_touch_slop_square_)
          always_in_bigger_tap_region_ = false;
      } else if (std::abs(scroll_x) > kScrollEpsilon ||
                 std::abs(scroll_y) > kScrollEpsilon) {
        handled = listener_->OnScroll(
            *current_down_event_, ev,
            (maximum_pointer_count_ > 1 && secondary_pointer_down_event_)
                ? *secondary_pointer_down_event_
                : ev,
            scroll_x, scroll_y);
        last_focus_x_ = focus_x;
        last_focus_y_ = focus_y;
      }

      if (!two_finger_tap_allowed_for_gesture_)
        break;

      // Two-finger tap should be prevented if either pointer exceeds its
      // (independent) slop region.
      // If the event has had more than two pointers down at any time,
      // two finger tap should be prevented.
      if (maximum_pointer_count_ > 2 || !IsWithinTouchSlop(ev)) {
        two_finger_tap_allowed_for_gesture_ = false;
      }
    } break;

    case MotionEvent::Action::UP:
      still_down_ = false;
      {
        if (is_double_tapping_ && should_process_double_tap) {
          // Finally, give the up event of the double-tap.
          CR_DCHECK(double_tap_listener_);
          handled |= double_tap_listener_->OnDoubleTapEvent(ev);
        } else if (all_pointers_within_slop_regions_ &&
                   maximum_pointer_count_ == 1) {
          if (is_down_candidate_for_repeated_single_tap_) {
            current_single_tap_repeat_count_ =
                (1 + current_single_tap_repeat_count_) %
                single_tap_repeat_interval_;
          } else {
            current_single_tap_repeat_count_ = 0;
          }
          handled = listener_->OnSingleTapUp(
              ev, 1 + current_single_tap_repeat_count_);
          if (defer_confirm_single_tap_ && should_process_double_tap &&
              double_tap_listener_ != NULL) {
            double_tap_listener_->OnSingleTapConfirmed(ev);
          }
        } else if (!all_pointers_within_slop_regions_) {
          // A fling must travel the minimum tap distance.
          current_single_tap_repeat_count_ = 0;
          const int pointer_id = ev.GetPointerId(0);
          velocity_tracker_.ComputeCurrentVelocity(1000, max_fling_velocity_);
          const float velocity_y = velocity_tracker_.GetYVelocity(pointer_id);
          const float velocity_x = velocity_tracker_.GetXVelocity(pointer_id);

          if ((std::abs(velocity_y) > min_fling_velocity_) ||
              (std::abs(velocity_x) > min_fling_velocity_)) {
            handled = listener_->OnFling(*current_down_event_, ev, velocity_x,
                                         velocity_y);
          }

          handled |= HandleSwipeIfNeeded(ev, velocity_x, velocity_y);
        }

        previous_up_event_ = ev.Clone();

        velocity_tracker_.Clear();
        is_double_tapping_ = false;
        defer_confirm_single_tap_ = false;
        timeout_handler_->StopTimeout(SHOW_PRESS);
        timeout_handler_->StopTimeout(LONG_PRESS);
      }
      maximum_pointer_count_ = 0;
      break;

    case MotionEvent::Action::CANCEL:
      Cancel();
      break;
  }

  return handled;
}

void GestureDetector::SetDoubleTapListener(
    DoubleTapListener* double_tap_listener) {
  if (double_tap_listener == double_tap_listener_)
    return;

  CR_DCHECK(!is_double_tapping_);

  // Null'ing the double-tap listener should flush an active tap timeout.
  if (!double_tap_listener) {
    if (timeout_handler_->HasTimeout(TAP)) {
      timeout_handler_->StopTimeout(TAP);
      OnTapTimeout();
    }
  }

  double_tap_listener_ = double_tap_listener;
}

void GestureDetector::Init(const Config& config) {
  CR_DCHECK(listener_);

  // Using a small epsilon when comparing slop distances allows pixel
  // perfect slop determination when using fractional DIP coordinates
  // (assuming the slop region and DPI scale are reasonably
  // proportioned).
  const float kSlopEpsilon = .05f;

  const float touch_slop = config.touch_slop + kSlopEpsilon;
  const float double_tap_touch_slop = touch_slop;
  const float double_tap_slop = config.double_tap_slop + kSlopEpsilon;
  touch_slop_square_ = touch_slop * touch_slop;
  double_tap_touch_slop_square_ = double_tap_touch_slop * double_tap_touch_slop;
  double_tap_slop_square_ = double_tap_slop * double_tap_slop;
  double_tap_timeout_ = config.double_tap_timeout;
  double_tap_min_time_ = config.double_tap_min_time;
  CR_DCHECK(double_tap_min_time_ < double_tap_timeout_);
  min_fling_velocity_ = config.minimum_fling_velocity;
  max_fling_velocity_ = config.maximum_fling_velocity;

  swipe_enabled_ = config.swipe_enabled;
  min_swipe_velocity_ = config.minimum_swipe_velocity;
  CR_DCHECK(config.maximum_swipe_deviation_angle > 0);
  CR_DCHECK(config.maximum_swipe_deviation_angle <= 45);
  const float maximum_swipe_deviation_angle =
      cr::ClampToRange(config.maximum_swipe_deviation_angle, 0.001f, 45.0f);
  min_swipe_direction_component_ratio_ =
      1.f / tan(gfx::DegToRad(maximum_swipe_deviation_angle));

  two_finger_tap_enabled_ = config.two_finger_tap_enabled;
  two_finger_tap_distance_square_ = config.two_finger_tap_max_separation *
                                    config.two_finger_tap_max_separation;
  two_finger_tap_timeout_ = config.two_finger_tap_timeout;

  CR_DCHECK(config.single_tap_repeat_interval >= 1);
  single_tap_repeat_interval_ = config.single_tap_repeat_interval;
}

void GestureDetector::OnShowPressTimeout() {
  listener_->OnShowPress(*current_down_event_);
}

void GestureDetector::OnLongPressTimeout() {
  timeout_handler_->StopTimeout(TAP);
  defer_confirm_single_tap_ = false;
  listener_->OnLongPress(*current_down_event_);
}

void GestureDetector::OnTapTimeout() {
  if (!double_tap_listener_)
    return;
  if (!still_down_) {
    CR_CHECK(previous_up_event_);
    double_tap_listener_->OnSingleTapConfirmed(*previous_up_event_);
  } else {
    defer_confirm_single_tap_ = true;
  }
}

void GestureDetector::Cancel() {
  // Stop waiting for a second tap and send a GESTURE_TAP_CANCEL to keep the
  // gesture stream valid.
  if (timeout_handler_->HasTimeout(TAP))
    listener_->OnTapCancel(*current_down_event_);
  CancelTaps();
  velocity_tracker_.Clear();
  all_pointers_within_slop_regions_ = false;
  still_down_ = false;
}

void GestureDetector::CancelTaps() {
  timeout_handler_->Stop();
  is_double_tapping_ = false;
  always_in_bigger_tap_region_ = false;
  defer_confirm_single_tap_ = false;
  is_down_candidate_for_repeated_single_tap_ = false;
  current_single_tap_repeat_count_ = 0;
}

bool GestureDetector::IsRepeatedTap(const MotionEvent& first_down,
                                    const MotionEvent& first_up,
                                    const MotionEvent& second_down,
                                    bool should_process_double_tap) const {
  if (!always_in_bigger_tap_region_)
    return false;

  const cr::TimeDelta delta_time =
      second_down.GetEventTime() - first_up.GetEventTime();
  if (delta_time > double_tap_timeout_)
    return false;

  // Only use the min time when in double-tap detection mode. For repeated
  // single taps the risk of accidental repeat detection (e.g., from fingernail
  // interference) is minimal.
  if (should_process_double_tap && double_tap_listener_ &&
      delta_time < double_tap_min_time_) {
    return false;
  }

  const float delta_x = first_down.GetX() - second_down.GetX();
  const float delta_y = first_down.GetY() - second_down.GetY();
  return (delta_x * delta_x + delta_y * delta_y < double_tap_slop_square_);
}

bool GestureDetector::HandleSwipeIfNeeded(const MotionEvent& up,
                                          float vx,
                                          float vy) {
  if (!swipe_enabled_ || (!vx && !vy))
    return false;
  float vx_abs = std::abs(vx);
  float vy_abs = std::abs(vy);

  if (vx_abs < min_swipe_velocity_)
    vx_abs = vx = 0;
  if (vy_abs < min_swipe_velocity_)
    vy_abs = vy = 0;

  // Note that the ratio will be 0 if both velocites are below the min.
  float ratio = vx_abs > vy_abs ? vx_abs / std::max(vy_abs, 0.001f)
                                : vy_abs / std::max(vx_abs, 0.001f);

  if (ratio < min_swipe_direction_component_ratio_)
    return false;

  if (vx_abs > vy_abs)
    vy = 0;
  else
    vx = 0;
  return listener_->OnSwipe(*current_down_event_, up, vx, vy);
}

bool GestureDetector::IsWithinTouchSlop(const MotionEvent& ev) {
  // If there have been more than two down pointers in the touch sequence,
  // tapping is not possible. Slop region check is not needed.
  if (maximum_pointer_count_ > 2)
    return false;

  for (size_t i = 0; i < ev.GetPointerCount(); i++) {
    const int pointer_id = ev.GetPointerId(i);
    const MotionEvent* source_pointer_down_event = GetSourcePointerDownEvent(
        *current_down_event_.get(), secondary_pointer_down_event_.get(),
        pointer_id);

    if (!source_pointer_down_event)
      return false;

    int source_index =
        source_pointer_down_event->FindPointerIndexOfId(pointer_id);
    CR_DCHECK(source_index >= 0);
    if (source_index < 0)
      return false;

    float dx = source_pointer_down_event->GetX(source_index) - ev.GetX(i);
    float dy = source_pointer_down_event->GetY(source_index) - ev.GetY(i);
    if (dx * dx + dy * dy > touch_slop_square_)
      return false;
  }

  return true;
}

const MotionEvent* GestureDetector::GetSourcePointerDownEvent(
    const MotionEvent& current_down_event,
    const MotionEvent* secondary_pointer_down_event,
    const int pointer_id) {
  if (current_down_event.GetPointerId(0) == pointer_id)
    return &current_down_event;

  // Secondary pointer down event is sometimes missing (crbug.com/704426), the
  // source pointer down event is not found in these cases.
  // crbug.com/704426 is the only related bug report and we don't have any
  // reliable repro of the bug.
  if (!secondary_pointer_down_event)
    return nullptr;

  for (size_t i = 0; i < secondary_pointer_down_event->GetPointerCount(); i++) {
    if (secondary_pointer_down_event->GetPointerId(i) == pointer_id)
      return secondary_pointer_down_event;
  }

  return nullptr;
}

}  // namespace crui

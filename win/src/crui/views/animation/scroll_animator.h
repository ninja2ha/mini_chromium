// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_ANIMATION_SCROLL_ANIMATOR_H_
#define UI_VIEWS_ANIMATION_SCROLL_ANIMATOR_H_

#include <memory>

#include "crui/gfx/animation/animation_delegate.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace gfx {
class SlideAnimation;
}  // namespace gfx

namespace views {

class CRUI_EXPORT ScrollDelegate {
 public:
  // Returns true if the content was actually scrolled, false otherwise.
  virtual bool OnScroll(float dx, float dy) = 0;

 protected:
  ~ScrollDelegate() = default;
};

class CRUI_EXPORT ScrollAnimator : public gfx::AnimationDelegate {
 public:
  ScrollAnimator(const ScrollAnimator&) = delete;
  ScrollAnimator& operator=(const ScrollAnimator&) = delete;

  // The ScrollAnimator does not own the delegate. Uses default acceleration.
  explicit ScrollAnimator(ScrollDelegate* delegate);
  ~ScrollAnimator() override;

  // Use this if you would prefer different acceleration than the default.
  void set_acceleration(float acceleration) { acceleration_ = acceleration; }

  void Start(float velocity_x, float velocity_y);
  void Stop();

  bool is_scrolling() const { return !!animation_.get(); }

 private:
  // Implementation of gfx::AnimationDelegate.
  void AnimationEnded(const gfx::Animation* animation) override;
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationCanceled(const gfx::Animation* animation) override;

  ScrollDelegate* delegate_;

  float velocity_x_;
  float velocity_y_;
  float last_t_;
  float duration_;
  float acceleration_;

  std::unique_ptr<gfx::SlideAnimation> animation_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_ANIMATION_SCROLL_ANIMATOR_H_

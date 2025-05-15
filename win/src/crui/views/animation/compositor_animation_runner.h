// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_ANIMATION_COMPOSITOR_ANIMATION_RUNNER_H_
#define UI_VIEWS_ANIMATION_COMPOSITOR_ANIMATION_RUNNER_H_

#include "crbase/scoped_observer.h"
#include "crbase/time/time.h"
///#include "crui/compositor/compositor.h"
///#include "crui/compositor/compositor_animation_observer.h"
///#include "crui/compositor/compositor_observer.h"
#include "crui/gfx/animation/animation_container.h"
#include "crui/views/widget/widget_observer.h"

namespace crui {
namespace views {
class Widget;

// An animation runner based on ui::Compositor.
class CompositorAnimationRunner : public gfx::AnimationRunner,
                                  ///public crui::CompositorAnimationObserver,
                                  public WidgetObserver {
 public:
  explicit CompositorAnimationRunner(Widget* widget);
  CompositorAnimationRunner(CompositorAnimationRunner&) = delete;
  CompositorAnimationRunner& operator=(CompositorAnimationRunner&) = delete;
  ~CompositorAnimationRunner() override;

  // gfx::AnimationRunner:
  void Stop() override;

  // ui::CompositorAnimationObserver:
  ///void OnAnimationStep(cr::TimeTicks timestamp) override;
  ///void OnCompositingShuttingDown(crui::Compositor* compositor) override;

  // WidgetObserver:
  void OnWidgetDestroying(Widget* widget) override;

 protected:
  // gfx::AnimationRunner:
  void OnStart(cr::TimeDelta min_interval, cr::TimeDelta elapsed) override;

 private:
  // When |widget_| is nullptr, it means the widget has been destroyed and
  // |compositor_| must also be nullptr.
  Widget* widget_;

  // When |compositor_| is nullptr, it means either the animation is not
  // running, or the compositor or |widget_| associated with the compositor_ has
  // been destroyed during animation.
  ///ui::Compositor* compositor_ = nullptr;

  cr::TimeDelta min_interval_ = cr::TimeDelta::Max();
  cr::TimeTicks last_tick_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_ANIMATION_COMPOSITOR_ANIMATION_RUNNER_H_

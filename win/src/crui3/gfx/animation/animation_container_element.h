// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_ANIMATION_ANIMATION_CONTAINER_ELEMENT_H_
#define UI_GFX_ANIMATION_ANIMATION_CONTAINER_ELEMENT_H_

#include "crbase/time/time.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace gfx {

// Interface for the elements the AnimationContainer contains. This is
// implemented by Animation.
class CRUI_EXPORT AnimationContainerElement {
 public:
  // Sets the start of the animation. This is invoked from
  // AnimationContainer::Start.
  virtual void SetStartTime(cr::TimeTicks start_time) = 0;

  // Invoked when the animation is to progress.
  virtual void Step(cr::TimeTicks time_now) = 0;

  // Returns the time interval of the animation. If an Element needs to change
  // this it should first invoke Stop, then Start.
  virtual cr::TimeDelta GetTimerInterval() const = 0;

 protected:
  virtual ~AnimationContainerElement() {}
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_ANIMATION_ANIMATION_CONTAINER_ELEMENT_H_

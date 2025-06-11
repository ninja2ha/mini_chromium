// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/animation/animation.h"

#include <windows.h>

#include "crbase/win/win_util.h"

namespace crui {
namespace gfx {

// static
bool Animation::ShouldRenderRichAnimationImpl() {
  BOOL result;
  // Get "Turn off all unnecessary animations" value.
  if (::SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &result, 0)) {
    return !!result;
  }
  return !cr::win::IsCurrentSessionRemote();
}

// static
bool Animation::ScrollAnimationsEnabledBySystem() {
  return ShouldRenderRichAnimation();
}

// static
void Animation::UpdatePrefersReducedMotion() {
  // prefers_reduced_motion_ should only be modified on the UI thread.
  // TODO(crbug.com/927163): DCHECK this assertion once tests are well-behaved.

  // We default to assuming that animations are enabled, to avoid impacting the
  // experience for users on systems that don't have SPI_GETCLIENTAREAANIMATION.
  BOOL win_anim_enabled = true;
  SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &win_anim_enabled, 0);
  prefers_reduced_motion_ = !win_anim_enabled;
}

} // namespace gfx
} // namespace crui

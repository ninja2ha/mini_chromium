// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_WINDOW_TREE_HOST_OBSERVER_H_
#define UI_AURA_WINDOW_TREE_HOST_OBSERVER_H_

#include "crui/base/ui_export.h"
#include "crui/aura/window.h"

namespace crui {

namespace gfx {
class Point;
}  // namespace gfx

namespace aura {
class WindowTreeHost;

class CRUI_EXPORT WindowTreeHostObserver {
 public:
  // Called when the host's client size has changed.
  virtual void OnHostResized(WindowTreeHost* host) {}

  // Called when the host is moved on screen.
  virtual void OnHostMovedInPixels(WindowTreeHost* host,
                                   const gfx::Point& new_origin_in_pixels) {}

  // Called when the host is moved to a different workspace.
  virtual void OnHostWorkspaceChanged(WindowTreeHost* host) {}

  // Called when the native window system sends the host request to close.
  virtual void OnHostCloseRequested(WindowTreeHost* host) {}

  // Called when the occlusion status of the native window changes, iff
  // occlusion tracking is enabled for a descendant of the root.
  virtual void OnOcclusionStateChanged(WindowTreeHost* host,
                                       Window::OcclusionState new_state) {}

  // Called before processing a bounds change. The bounds change may result in
  // one or both of OnHostResized() and OnHostMovedInPixels() being called.
  // This is not supported by all WindowTreeHosts.
  // OnHostWillProcessBoundsChange() is always followed by
  // OnHostDidProcessBoundsChange().
  virtual void OnHostWillProcessBoundsChange(WindowTreeHost* host) {}
  virtual void OnHostDidProcessBoundsChange(WindowTreeHost* host) {}

 protected:
  virtual ~WindowTreeHostObserver() {}
};

}  // namespace aura

}  // namespace crui

#endif  // UI_AURA_WINDOW_TREE_HOST_OBSERVER_H_

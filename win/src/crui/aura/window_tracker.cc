// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/window_tracker.h"

#include "crui/aura/window.h"

namespace crui {
namespace aura {

WindowTracker::WindowTracker(const WindowList& windows) {
  for (Window* window : windows)
    Add(window);
}

WindowTracker::WindowTracker() = default;

WindowTracker::~WindowTracker() {
  RemoveAll();
}

void WindowTracker::Add(Window* window) {
  if (cr::Contains(windows_, window))
    return;

  window->AddObserver(this);
  windows_.push_back(window);
}

void WindowTracker::RemoveAll() {
  for (Window* window : windows_)
    window->RemoveObserver(this);
  windows_.clear();
}

void WindowTracker::Remove(Window* window) {
  auto iter = std::find(windows_.begin(), windows_.end(), window);
  if (iter != windows_.end()) {
    window->RemoveObserver(this);
    windows_.erase(iter);
  }
}

Window* WindowTracker::Pop() {
  CR_DCHECK(!windows_.empty());
  Window* result = windows_[0];
  Remove(result);
  return result;
}

bool WindowTracker::Contains(Window* window) const {
  return cr::Contains(windows_, window);
}

void WindowTracker::OnWindowDestroying(Window* window) {
  CR_DCHECK(Contains(window));
  Remove(window);
}

}  // namespace aura
}  // namespace crui

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WINDOW_NATIVE_FRAME_VIEW_H_
#define UI_VIEWS_WINDOW_NATIVE_FRAME_VIEW_H_

#include "crui/views/window/non_client_view.h"

namespace crui {
namespace views {

class Widget;

class CRUI_EXPORT NativeFrameView : public NonClientFrameView {
 public:
  static const char kViewClassName[];

  NativeFrameView(const NativeFrameView&) = delete;
  NativeFrameView& operator=(const NativeFrameView&) = delete;

  explicit NativeFrameView(Widget* frame);
  ~NativeFrameView() override;

  // NonClientFrameView overrides:
  gfx::Rect GetBoundsForClientView() const override;
  gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const override;
  int NonClientHitTest(const gfx::Point& point) override;
  ///void GetWindowMask(const gfx::Size& size, SkPath* window_mask) override;
  void ResetWindowControls() override;
  void UpdateWindowIcon() override;
  void UpdateWindowTitle() override;
  void SizeConstraintsChanged() override;

  // View overrides:
  gfx::Size CalculatePreferredSize() const override;
  gfx::Size GetMinimumSize() const override;
  gfx::Size GetMaximumSize() const override;
  const char* GetClassName() const override;

 private:
  // Our containing frame.
  Widget* frame_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WINDOW_NATIVE_FRAME_VIEW_H_

// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/corewm/tooltip_win.h"

#include <windowsx.h>

#include "crbase/logging.h"
#include "crui/base/i18n/rtl.h"
///#include "ui/base/l10n/l10n_util_win.h"
#include "crui/display/display.h"
#include "crui/display/screen.h"
#include "crui/display/win/screen_win.h"
#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/font/system_fonts_win.h"
#include "crui/views/corewm/cursor_height_provider_win.h"

namespace crui {
namespace views {
namespace corewm {

TooltipWin::TooltipWin(HWND parent)
    : parent_hwnd_(parent), tooltip_hwnd_(nullptr), showing_(false) {
  memset(&toolinfo_, 0, sizeof(toolinfo_));
  toolinfo_.cbSize = sizeof(toolinfo_);
  toolinfo_.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
  toolinfo_.uId = reinterpret_cast<UINT_PTR>(parent_hwnd_);
  toolinfo_.hwnd = parent_hwnd_;
  toolinfo_.lpszText = nullptr;
  toolinfo_.lpReserved = nullptr;
  SetRectEmpty(&toolinfo_.rect);
}

TooltipWin::~TooltipWin() {
  if (tooltip_hwnd_)
    DestroyWindow(tooltip_hwnd_);
}

bool TooltipWin::HandleNotify(int w_param, NMHDR* l_param, LRESULT* l_result) {
  if (tooltip_hwnd_ == nullptr)
    return false;

  switch (l_param->code) {
    case TTN_POP:
      showing_ = false;
      return true;
    case TTN_SHOW:
      *l_result = TRUE;
      PositionTooltip();
      showing_ = true;
      return true;
    default:
      break;
  }
  return false;
}

bool TooltipWin::EnsureTooltipWindow() {
  if (tooltip_hwnd_)
    return true;

  tooltip_hwnd_ =
      ::CreateWindowExW(WS_EX_TRANSPARENT, ///| l10n_util::GetExtendedTooltipStyles(),
                        TOOLTIPS_CLASS, nullptr, TTS_NOPREFIX | WS_POPUP, 0, 0, 0,
                        0, parent_hwnd_, nullptr, nullptr, nullptr);
  if (!tooltip_hwnd_) {
    CR_PLOG(Warning) << "tooltip creation failed, disabling tooltips";
    return false;
  }

  MaybeOverrideFont();

  ::SendMessageW(tooltip_hwnd_, TTM_ADDTOOL, 0,
                 reinterpret_cast<LPARAM>(&toolinfo_));
  return true;
}

void TooltipWin::PositionTooltip() {
  gfx::Point screen_point =
      display::win::ScreenWin::DIPToScreenPoint(location_);
  const int cursoroffset = GetCurrentCursorVisibleHeight();
  screen_point.Offset(0, cursoroffset);

  DWORD tooltip_size = static_cast<DWORD>(::SendMessageW(
      tooltip_hwnd_, TTM_GETBUBBLESIZE, 0,
      reinterpret_cast<LPARAM>(&toolinfo_)));
  const gfx::Size size(LOWORD(tooltip_size), HIWORD(tooltip_size));

  const display::Display display(
      display::Screen::GetScreen()->GetDisplayNearestPoint(location_));

  gfx::Rect tooltip_bounds(screen_point, size);
  tooltip_bounds.AdjustToFit(
      display::win::ScreenWin::DIPToScreenRect(parent_hwnd_,
                                               display.work_area()));
  SetWindowPos(tooltip_hwnd_, nullptr, tooltip_bounds.x(), tooltip_bounds.y(),
               0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

  MaybeOverrideFont();
}

void TooltipWin::MaybeOverrideFont() {
  gfx::win::FontAdjustment font_adjustment;
  const HFONT old_font = GetWindowFont(tooltip_hwnd_);

  // Determine if we need to override the font.
  ///if ((!override_font_.get() || override_font_.get() != old_font) &&
  ///    l10n_util::NeedOverrideDefaultUIFont(
  ///        &font_adjustment.font_family_override, &font_adjustment.font_scale)) {
  ///  // Determine if we need to regenerate the font.
  ///  // There are a number of situations under which Windows can replace the
  ///  // font in a tooltip, but we don't actually need to regenerate our override
  ///  // font unless the underlying text/DPI scale of the window has changed.
  ///  const float current_scale =
  ///      display::win::ScreenWin::GetScaleFactorForHWND(tooltip_hwnd_);
  ///  if (!override_font_.get() || current_scale != override_scale_) {
  ///    override_font_.reset(
  ///        gfx::win::AdjustExistingSystemFont(old_font, font_adjustment));
  ///    override_scale_ = current_scale;
  ///  }
  ///
  ///  // Override the font in the tooltip.
  ///  SetWindowFont(tooltip_hwnd_, override_font_.get(), FALSE);
  ///}
}

int TooltipWin::GetMaxWidth(const gfx::Point& location) const {
  const gfx::Point screen_point =
      display::win::ScreenWin::DIPToScreenPoint(location);
  display::Display display(
      display::Screen::GetScreen()->GetDisplayNearestPoint(screen_point));
  const gfx::Rect monitor_bounds = display.bounds();
  return (monitor_bounds.width() + 1) / 2;
}

void TooltipWin::SetText(aura::Window* window,
                         const cr::string16& tooltip_text,
                         const gfx::Point& location) {
  if (!EnsureTooltipWindow())
    return;

  // See comment in header for details on why |location_| is needed.
  location_ = location;

  cr::string16 adjusted_text(tooltip_text);
  ///cr::i18n::AdjustStringForLocaleDirection(&adjusted_text);
  toolinfo_.lpszText = const_cast<WCHAR*>(adjusted_text.c_str());
  ::SendMessageW(tooltip_hwnd_, TTM_SETTOOLINFO, 0,
                 reinterpret_cast<LPARAM>(&toolinfo_));

  int max_width = GetMaxWidth(location_);
  ::SendMessageW(tooltip_hwnd_, TTM_SETMAXTIPWIDTH, 0, max_width);
}

void TooltipWin::Show() {
  if (!EnsureTooltipWindow())
    return;

  ::SendMessageW(tooltip_hwnd_, TTM_TRACKACTIVATE,
                 TRUE, reinterpret_cast<LPARAM>(&toolinfo_));

  // Bring the window to the front.
  SetWindowPos(tooltip_hwnd_, HWND_TOPMOST, 0, 0, 0, 0,
               SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE);
}

void TooltipWin::Hide() {
  if (!tooltip_hwnd_)
    return;

  ::SendMessageW(tooltip_hwnd_, TTM_TRACKACTIVATE, FALSE,
                 reinterpret_cast<LPARAM>(&toolinfo_));
}

bool TooltipWin::IsVisible() {
  return showing_;
}

}  // namespace corewm
}  // namespace views
}  // namespace crui

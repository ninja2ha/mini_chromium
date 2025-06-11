// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_SYSTEM_FONTS_WIN_H_
#define UI_GFX_SYSTEM_FONTS_WIN_H_

#include "crui/gfx/font/font.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace gfx {
namespace win {

// Represents an optional override of system font and scale.
struct FontAdjustment {
  cr::string16 font_family_override;
  double font_scale = 1.0;
};

// Identifiers for Windows-specific fonts described in the NONCLIENTMETRICS
// struct.
enum class SystemFont { kCaption = 0, kSmallCaption, kMenu, kStatus, kMessage };

// Callback that returns the minimum height that should be used for
// gfx::Fonts. Optional. If not specified, the minimum font size is 0.
typedef int (*GetMinimumFontSizeCallback)();
CRUI_EXPORT void SetGetMinimumFontSizeCallback(
    GetMinimumFontSizeCallback callback);

// Callback that adjusts a FontAdjustment to meet suitability requirements
// of the embedding application. Optional. If not specified, no adjustments
// are performed other than clamping to a minimum font size if
// |get_minimum_font_size_callback| is specified.
typedef void (*AdjustFontCallback)(FontAdjustment* font_adjustment);
CRUI_EXPORT void SetAdjustFontCallback(AdjustFontCallback callback);

// Returns the specified Windows default system font. By default, this is the
// font used for message boxes (see struct NONCLIENTMETRICS).
CRUI_EXPORT const Font& GetDefaultSystemFont();

// Returns the specified Windows system font, suitable for drawing on screen
// elements.
CRUI_EXPORT const Font& GetSystemFont(SystemFont system_font);

// Applies a font adjustment to an existing native font.
CRUI_EXPORT NativeFont
AdjustExistingSystemFont(NativeFont existing_font,
                         const FontAdjustment& font_adjustment);

// Computes and returns the adjusted size of a font, subject to the global
// minimum size. |lf_height| is the height as reported by the LOGFONT structure,
// and may be positive or negative (but is typically negative, indicating
// character size rather than cell size). The absolute value of |lf_size| will
// be adjusted by |size_delta| and then returned with the original sign.
CRUI_EXPORT int AdjustFontSize(int lf_height, int size_delta);

// Adjusts a LOGFONT structure for optional size scale and face override.
CRUI_EXPORT void AdjustLOGFONTForTesting(const FontAdjustment& font_adjustment,
                                          LOGFONT* logfont);

// Retrieves a FONT from a LOGFONT structure.
CRUI_EXPORT Font GetFontFromLOGFONTForTesting(const LOGFONT& logfont);

// Clears the system fonts cache. SystemFonts is keeping a global state that
// must be reset in unittests when using |GetMinimumFontSizeCallback| or
// |SetAdjustFontCallback|.
CRUI_EXPORT void ResetSystemFontsForTesting();

}  // namespace win
}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_SYSTEM_FONTS_WIN_H_

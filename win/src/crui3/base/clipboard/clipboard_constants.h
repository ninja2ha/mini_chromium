// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CLIPBOARD_CLIPBOARD_CONSTANTS_H_
#define UI_BASE_CLIPBOARD_CLIPBOARD_CONSTANTS_H_

#include <string>

#include "crui/base/ui_export.h"
#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_MACOSX)
#ifdef __OBJC__
@class NSString;
#else
class NSString;
#endif
#endif  // defined(MINI_CHROMIUM_OS_MACOSX)

namespace crui {

// Platform-Neutral MIME type constants.
CRUI_EXPORT extern const char kMimeTypeText[];
CRUI_EXPORT extern const char kMimeTypeTextUtf8[];
CRUI_EXPORT extern const char kMimeTypeURIList[];
CRUI_EXPORT extern const char kMimeTypeDownloadURL[];
CRUI_EXPORT extern const char kMimeTypeMozillaURL[];
CRUI_EXPORT extern const char kMimeTypeHTML[];
CRUI_EXPORT extern const char kMimeTypeRTF[];
CRUI_EXPORT extern const char kMimeTypePNG[];

#if !defined(MINI_CHROMIUM_OS_MACOSX)
CRUI_EXPORT extern const char kMimeTypeWebCustomData[];
CRUI_EXPORT extern const char kMimeTypeWebkitSmartPaste[];
CRUI_EXPORT extern const char kMimeTypePepperCustomData[];
#else
// MacOS-specific Uniform Type Identifiers.

// Pickled data.
CRUI_EXPORT extern NSString* const kWebCustomDataPboardType;
// Tells us if WebKit was the last to write to the pasteboard. There's no
// actual data associated with this type.
CRUI_EXPORT extern NSString* const kWebSmartPastePboardType;
// Pepper custom data format type.
CRUI_EXPORT extern NSString* const kPepperCustomDataPboardType;
#endif  // defined(OS_MACOSX)

}  // namespace crui

#endif  // UI_BASE_CLIPBOARD_CLIPBOARD_H_

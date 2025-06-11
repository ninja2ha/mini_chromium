// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_PLATFORM_EVENT_H_
#define UI_EVENTS_PLATFORM_EVENT_H_

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#elif defined(MINI_CHROMIUM_USE_X11)
typedef union _XEvent XEvent;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
#if defined(__OBJC__)
@class NSEvent;
#else   // __OBJC__
class NSEvent;
#endif  // __OBJC__
#endif

namespace crui {
class Event;

// Cross platform typedefs for native event types.
#if defined(MINI_CHROMIUM_OS_WIN)
using PlatformEvent = MSG;
#elif defined(MINI_CHROMIUM_USE_X11)
using PlatformEvent = XEvent*;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
using PlatformEvent = NSEvent*;
#else
using PlatformEvent = void*;
#endif

}  // namespace crui

#endif  // UI_EVENTS_PLATFORM_EVENT_H_

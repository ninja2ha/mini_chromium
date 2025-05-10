// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRUI_BASE_UI_BUILD_PLATFORM_H_
#define MINI_CHROMIUM_SRC_CRUI_BASE_UI_BUILD_PLATFORM_H_

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN) 
///#define MINI_CHROMIUM_USE_AURA 1
#elif defined(MINI_CHROMIUM_OS_LINUX)
#define MINI_CHROMIUM_USE_AURA 1
#define MINI_CHROMIUM_USER_X11 1
#elif defined(MINI_CHROMIUM_OS_MACOSX)
///#define MINI_CHROMIUM_USER_X11 1
#else
#error unknown platform
#endif

#endif // !MINI_CHROMIUM_SRC_CRUI_BASE_UI_BUILD_PLATFORM_H_

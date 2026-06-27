// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_FULL_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_FULL_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

typedef struct IUnknown IUnknown;
#include <windows.h>

// com
#include <objbase.h>
#include <oleauto.h>
#include <unknwn.h>
#include <propidl.h>

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_FULL_H_
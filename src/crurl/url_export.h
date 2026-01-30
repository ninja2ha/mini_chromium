// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRGURL_URL_EXPORT_H_
#define MINI_CHROMIUM_SRC_CRGURL_URL_EXPORT_H_

#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_COMPONENT_BUILD)
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#pragma warning(disable: 4251)

#if defined(CRURL_IMPLEMENTATION)
#define CRURL_EXPORT __declspec(dllexport)
#else
#define CRURL_EXPORT __declspec(dllimport)
#endif  // defined(CRURL_IMPLEMENTATION)

#else  // defined(CRURL_IMPLEMENTATION)
#if defined(CRURL_IMPLEMENTATION)
#define CRURL_EXPORT __attribute__((visibility("default")))
#else
#define CRURL_EXPORT
#endif  // defined(CRURL_IMPLEMENTATION)
#endif

#else  // defined(MINI_CHROMIUM_COMPONENT_BUILD)
#define CRURL_EXPORT
#endif

#endif  // MINI_CHROMIUM_SRC_CRGURL_URL_EXPORT_H_
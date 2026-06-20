// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRWINRT_EXPORT_H_
#define MINI_CHROMIUM_SRC_CRWINRT_EXPORT_H_

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_COMPONENT_BUILD)
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#pragma warning(disable: 4251)

#if defined(CRWINRT_IMPLEMENTATION)
#define CRWINRT_EXPORT __declspec(dllexport)
#else
#define CRWINRT_EXPORT __declspec(dllimport)
#endif  // defined(CRWINRT_IMPLEMENTATION)

#else  // defined(CRWINRT_IMPLEMENTATION)
#if defined(CRWINRT_IMPLEMENTATION)
#define CRWINRT_EXPORT __attribute__((visibility("default")))
#else
#define CRWINRT_EXPORT
#endif  // defined(CRWINRT_IMPLEMENTATION)
#endif

#else  // defined(MINI_CHROMIUM_COMPONENT_BUILD)
#define CRWINRT_EXPORT
#endif

#endif  // MINI_CHROMIUM_SRC_CRWINRT_EXPORT_H_
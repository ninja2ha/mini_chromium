// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CR_WINNT_EXPORT_H_
#define MINI_CHROMIUM_SRC_CR_WINNT_EXPORT_H_

#include "cr_build/build_config.h"

#if defined(MINI_CHROMIUM_COMPONENT_BUILD)
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#pragma warning(disable: 4251)

#if defined(CRWINNT_IMPLEMENTATION)
#define CRWINNT_EXPORT __declspec(dllexport)
#else
#define CRWINNT_EXPORT __declspec(dllimport)
#endif  // defined(CRWINNT_IMPLEMENTATION)

#else  // defined(CRWINNT_IMPLEMENTATION)
#if defined(CRWINNT_IMPLEMENTATION)
#define CRWINNT_EXPORT __attribute__((visibility("default")))
#else
#define CRWINNT_EXPORT
#endif  // defined(CRWINNT_IMPLEMENTATION)
#endif

#else  // defined(MINI_CHROMIUM_COMPONENT_BUILD)
#define CRWINNT_EXPORT
#endif

#endif  // MINI_CHROMIUM_SRC_CR_WINNT_EXPORT_H_
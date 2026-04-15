// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRIPC_IPC_EXPORT_H_
#define MINI_CHROMIUM_SRC_CRIPC_IPC_EXPORT_H_

#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_COMPONENT_BUILD)
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#pragma warning(disable: 4251)

#if defined(CRIPC_IMPLEMENTATION)
#define CRIPC_EXPORT __declspec(dllexport)
#else
#define CRIPC_EXPORT __declspec(dllimport)
#endif  // defined(CRIPC_IMPLEMENTATION)

#else  // defined(MINI_CHROMIUM_COMPILER_MSVC)
#if defined(CRIPC_IMPLEMENTATION)
#define CRIPC_EXPORT __attribute__((visibility("default")))
#else
#define CRIPC_EXPORT
#endif  // defined(CRIPC_IMPLEMENTATION)
#endif

#else  // defined(CRIPC_IMPLEMENTATION)
#define CRIPC_EXPORT
#endif

#endif  // MINI_CHROMIUM_SRC_CRIPC_IPC_EXPORT_H_
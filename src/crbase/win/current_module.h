// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_CURRENT_MODULE_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_CURRENT_MODULE_H_

#include "crbase/win/windows_types.h"

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;

// Returns the HMODULE of the dll the macro was expanded in.
// Only use in cc files, not in h files.
#define CR_CURRENT_MODULE() reinterpret_cast<HMODULE>(&__ImageBase)

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_CURRENT_MODULE_H_
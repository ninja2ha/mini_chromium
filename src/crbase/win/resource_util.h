// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

// This file contains utility functions for accessing resources in external
// files (DLLs) or embedded in the executable itself.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_RESOURCE_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_RESOURCE_UTIL_H_

#include <stddef.h>

#include "crbase/base_export.h"
#include "crbase/win/windows_types.h"

namespace cr {
namespace win {

// Function for getting a data resource of the specified |resource_type| from
// a dll.  Some resources are optional, especially in unit tests, so this
// returns false but doesn't raise an error if the resource can't be loaded.
bool CRBASE_EXPORT GetResourceFromModule(HMODULE module,
                                         int resource_id,
                                         LPCWSTR resource_type,
                                         void** data,
                                         size_t* length);

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_RESOURCE_UTIL_H_

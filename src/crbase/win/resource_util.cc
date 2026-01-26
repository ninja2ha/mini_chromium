// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/win/resource_util.h"
#include "crbase/logging/logging.h"

namespace cr {
namespace win {

bool GetResourceFromModule(HMODULE module,
                           int resource_id,
                           LPCWSTR resource_type,
                           void** data,
                           size_t* length) {
  if (!module)
    return false;

  if (!IS_INTRESOURCE(resource_id)) {
    CR_NOTREACHED();
    return false;
  }

  HRSRC hres_info =
      ::FindResourceW(module, MAKEINTRESOURCEW(resource_id), resource_type);
  if (nullptr == hres_info)
    return false;

  DWORD data_size = SizeofResource(module, hres_info);
  HGLOBAL hres = LoadResource(module, hres_info);
  if (!hres)
    return false;

  void* resource = LockResource(hres);
  if (!resource)
    return false;

  *data = resource;
  *length = static_cast<size_t>(data_size);
  return true;
}

}  // namespace win
}  // namespace cr

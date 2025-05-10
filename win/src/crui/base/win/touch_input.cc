// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/win/touch_input.h"
#include "crbase/win/win_util.h"

namespace crui {

BOOL GetTouchInputInfoWrapper(HTOUCHINPUT handle,
                              UINT count,
                              PTOUCHINPUT pointer,
                              int size) {

  typedef BOOL(WINAPI* GetTouchInputInfoPtr)(
      HTOUCHINPUT, UINT, PTOUCHINPUT, int);
  static const auto get_touch_input_info_func =
      reinterpret_cast<GetTouchInputInfoPtr>(
          cr::win::GetUser32FunctionPointer("GetTouchInputInfo"));
  if (get_touch_input_info_func)
    return get_touch_input_info_func(handle, count, pointer, size);
  return FALSE;
}

}  // namespace crui

// Copyright 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/win/win_util.h"

#include "crbase/win/windows_types.h"
#include "crbase/win/win_api_helper.h"

namespace cr {
namespace win {

bool IsUser32AndGdi32Available() {
  static auto is_user32_and_gdi32_available = []() {
    // If win32k syscalls aren't disabled, then user32 and gdi32 are available.
    using GetProcessMitigationPolicyType = 
        BOOL(WINAPI*)(HANDLE, ULONG, PVOID, SIZE_T);
    GetProcessMitigationPolicyType get_process_mitigation_policy_func =
        reinterpret_cast<GetProcessMitigationPolicyType>(GetProcAddress(
            GetKernel32Module(), "GetProcessMitigationPolicy"));

    if (!get_process_mitigation_policy_func)
      return true;

    PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY policy = {};
    if (get_process_mitigation_policy_func(GetCurrentProcess(),
                                           ProcessSystemCallDisablePolicy,
                                           &policy, sizeof(policy))) {
      return policy.DisallowWin32kSystemCalls == 0;
    }

    return true;
  }();
  return is_user32_and_gdi32_available;
}

} // namespace win
} // namespace cr

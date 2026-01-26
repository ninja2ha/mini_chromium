// Copyright 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_

// Including this file to instead of <windows.h>.

#if defined(min)
#undef min
#endif  // max

#if defined(max)
#undef max
#endif  // max

#if !defined(NOMINMAX)
#define NOMINMAX
#endif  // NOMINMAX

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN

// Fixs error: C2440
#if defined(_MSC_VER) && defined(__cplusplus) && !defined(CINTERFACE)
extern "C++" {
  struct IUnknown;
}
#endif

#include <windows.h>

////////////////////////////////////////////////////////////////////////////////

#if !defined(MAKEINTATOMW)
#define MAKEINTATOMW(i) (LPWSTR)((ULONG_PTR)((WORD)(i)))
#endif

////////////////////////////////////////////////////////////////////////////////

#if _WIN32_WINNT < 0x603

// Copied from wint.h
typedef struct _PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY {
  union {
    DWORD Flags;
    struct {
      DWORD DisallowWin32kSystemCalls : 1;
      DWORD AuditDisallowWin32kSystemCalls : 1;
      DWORD DisallowFsctlSystemCalls : 1;
      DWORD AuditDisallowFsctlSystemCalls : 1;
      DWORD ReservedFlags : 28;
    } DUMMYSTRUCTNAME;
  } DUMMYUNIONNAME;
} PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY;

typedef enum _PROCESS_MITIGATION_POLICY {
  ProcessDEPPolicy,
  ProcessASLRPolicy,
  ProcessDynamicCodePolicy,
  ProcessStrictHandleCheckPolicy,
  ProcessSystemCallDisablePolicy,
  ProcessMitigationOptionsMask,
  ProcessExtensionPointDisablePolicy,
  ProcessReserved1Policy,
  ProcessSignaturePolicy,
  MaxProcessMitigationPolicy
} PROCESS_MITIGATION_POLICY, *PPROCESS_MITIGATION_POLICY;

#endif


#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_

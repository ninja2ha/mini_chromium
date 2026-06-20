// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOW_TYPES_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOW_TYPES_H_

#include "cr_base/compiler_config.h"

#if defined(__cplusplus)
extern "C" {
#endif

// -- ints ---------------------------------------------------------------------

typedef unsigned long DWORD;
typedef long LONG;
typedef long long LONGLONG;
typedef void *HANDLE;
typedef void *PVOID;

#if defined(MINI_CHROMIUM_ARCH_CPU_32_BITS)
typedef unsigned long ULONG_PTR, *PULONG_PTR;
#else
typedef unsigned long long ULONG_PTR, *PULONG_PTR;
#endif

// -- structure ----------------------------------------------------------------

#pragma pack(push, 8)
typedef struct _CR_CRITICAL_SECTION {
  PVOID DebugInfo;  // PRTL_CRITICAL_SECTION_DEBUG 

  //
  //  The following three fields control entering and exiting the critical
  //  section for the resource
  //

  LONG LockCount;
  LONG RecursionCount;
  HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
  HANDLE LockSemaphore;
  ULONG_PTR SpinCount;        // force size on 64-bit systems when packed
} CR_CRITICAL_SECTION, *PCR_CRITICAL_SECTION;
#pragma pack(pop)
typedef struct _RTL_CRITICAL_SECTION* PRTL_CRITICAL_SECTION;

///typedef struct _FILETIME {
///  DWORD dwLowDateTime;
///  DWORD dwHighDateTime;
///} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _FILETIME FILETIME;

// -- mascros ------------------------------------------------------------------

#ifndef TLS_OUT_OF_INDEXES
#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)
#endif

#if defined(__cplusplus)
}
#endif

// -- functions ----------------------------------------------------------------

namespace cr {
namespace win {

inline PRTL_CRITICAL_SECTION AsCriticalSestion(PCR_CRITICAL_SECTION ptr) {
  return reinterpret_cast<PRTL_CRITICAL_SECTION>(ptr);
}

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOW_TYPES_H_
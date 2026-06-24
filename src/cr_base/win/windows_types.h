// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_

#include "cr_base/compiler_config.h"
#include "cr_base/compiler_specific.h"

#if defined(__cplusplus)
extern "C" {
#endif

// -- mascros ------------------------------------------------------------------

#ifndef TLS_OUT_OF_INDEXES
#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)
#endif

// copied from system.
#ifndef CR_INVALID_HANDLE_VALUE
#define CR_INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif

// path length
#define CR_MAX_PATH (260)

// -- ints ---------------------------------------------------------------------

typedef wchar_t WCHAR;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef long long LONGLONG;
typedef unsigned __int64 ULONGLONG;

typedef void *HANDLE;
typedef void *PVOID;

#if defined(MINI_CHROMIUM_ARCH_CPU_32_BITS)
typedef __w64 int INT_PTR, *PINT_PTR;
typedef __w64 unsigned int UINT_PTR, *PUINT_PTR;

typedef __w64 long LONG_PTR, *PLONG_PTR;
typedef __w64 unsigned long ULONG_PTR, *PULONG_PTR;
#else
typedef __int64 INT_PTR, *PINT_PTR;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

typedef __int64 LONG_PTR, *PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#endif

typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK REGSAM;

// Forward declare Windows compatible handles.
#define CHROME_DECLARE_HANDLE(name) \
  struct name##__;                  \
  typedef struct name##__* name
CHROME_DECLARE_HANDLE(HINSTANCE);
CHROME_DECLARE_HANDLE(HKEY);
#undef CHROME_DECLARE_HANDLE

typedef HINSTANCE HMODULE;

// -- structure ----------------------------------------------------------------

// _WIN32_FIND_DATAW is 592 bytes and the largest built-in type in it is a
// DWORD. The buffer declaration guarantees the correct size and alignment.
struct CR_WIN32_FIND_DATAW {
  DWORD buffer[592 / sizeof(DWORD)];
};

struct CR_ALIGNAS(8) CR_CRITICAL_SECTION {
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
};

struct CR_PROCESSENTRY {
  DWORD   dwSize;
  DWORD   cntUsage;
  DWORD   th32ProcessID;          // this process
  ULONG_PTR th32DefaultHeapID;
  DWORD   th32ModuleID;           // associated exe
  DWORD   cntThreads;
  DWORD   th32ParentProcessID;    // this process's parent process
  LONG    pcPriClassBase;         // Base priority of process's threads
  DWORD   dwFlags;
  WCHAR   szExeFile[CR_MAX_PATH];    // Path
};
typedef struct tagPROCESSENTRY32W PROCESSENTRY32W;

// Use WIN32_FIND_DATAW when you just need a forward declaration. Use
// CR_WIN32_FIND_DATA when you need a concrete declaration to reserve
// space.
typedef struct _WIN32_FIND_DATAW WIN32_FIND_DATAW;
typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION;
typedef struct _FILETIME FILETIME;

#if defined(__cplusplus)
}
#endif

// -- functions ----------------------------------------------------------------

namespace cr {
namespace win {

inline RTL_CRITICAL_SECTION* ToWinType(CR_CRITICAL_SECTION* ptr) {
  return reinterpret_cast<RTL_CRITICAL_SECTION*>(ptr);
}

inline WIN32_FIND_DATAW* ToWinType(CR_WIN32_FIND_DATAW* ptr) {
  return reinterpret_cast<WIN32_FIND_DATAW*>(ptr);
}

inline const WIN32_FIND_DATAW* ToWinType(const CR_WIN32_FIND_DATAW* ptr) {
  return reinterpret_cast<const WIN32_FIND_DATAW*>(ptr);
}

inline PROCESSENTRY32W* ToWinType(CR_PROCESSENTRY* ptr) {
  return reinterpret_cast<PROCESSENTRY32W*>(ptr);
}

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_
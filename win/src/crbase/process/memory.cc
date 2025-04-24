// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/process/memory.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#endif  // defined(OS_WIN)

#include "crbase/logging.h"
#include "crbase/debug/alias.h"
#include "crbase/helper/stl_util.h"
#include "crbase/build_platform.h"

namespace cr {

namespace internal {

void OnNoMemoryInternal(size_t size) {
#if defined(MINI_CHROMIUM_OS_WIN)
  // Kill the process. This is important for security since most of code
  // does not check the result of memory allocation.
  // https://msdn.microsoft.com/en-us/library/het71c37.aspx
  // Pass the size of the failed request in an exception argument.
  ULONG_PTR exception_args[] = {size};
  ::RaiseException(cr::win::kOomExceptionCode, EXCEPTION_NONCONTINUABLE,
                   static_cast<DWORD>(cr::size(exception_args)), 
                   exception_args);

  // Safety check, make sure process exits here.
  _exit(win::kOomExceptionCode);
#else
  size_t tmp_size = size;
  base::debug::Alias(&tmp_size);
  LOG(FATAL) << "Out of memory. size=" << tmp_size;
#endif  // defined(OS_WIN)
}

}  // namespace internal

// Defined in memory_win.cc for Windows.
#if !defined(MINI_CHROMIUM_OS_WIN)

namespace {

// Breakpad server classifies base::`anonymous namespace'::OnNoMemory as
// out-of-memory crash.
NOINLINE void OnNoMemory(size_t size) {
  internal::OnNoMemoryInternal(size);
}

}  // namespace

void TerminateBecauseOutOfMemory(size_t size) {
  OnNoMemory(size);
}

#endif  // !defined(MINI_CHROMIUM_OS_WIN)

bool UncheckedCalloc(size_t num_items, size_t size, void** result) {
  const size_t alloc_size = num_items * size;

  // Overflow check
  if (size && ((alloc_size / size) != num_items)) {
    *result = nullptr;
    return false;
  }

  if (!UncheckedMalloc(alloc_size, result))
    return false;

  memset(*result, 0, alloc_size);
  return true;
}

namespace internal {
bool ReleaseAddressSpaceReservation() {
  return false;
}
}  // namespace internal

}  // namespace cr
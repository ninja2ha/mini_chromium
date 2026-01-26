// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_SCOPED_HANDLE_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_SCOPED_HANDLE_H_

#include "crbase/base_export.h"
#include "crbase/location.h"
#include "crbase/logging/logging.h"
#include "crbuild/compiler_specific.h"
#include "crbuild/build_config.h"

// TODO(rvargas): remove this with the rest of the verifier.
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#include "crbase/win/windows_types.h"
#include <intrin.h>
#define CRBASE_WIN_GET_CALLER _ReturnAddress()
#elif defined(MINI_CHROMIUM_COMPILER_GCC)
#define CRBASE_WIN_GET_CALLER \
  __builtin_extract_return_addr(\ __builtin_return_address(0))
#endif

namespace cr {
namespace win {

// Generic wrapper for raw handles that takes care of closing handles
// automatically. The class interface follows the style of
// the ScopedFILE class with two additions:
//   - IsValid() method can tolerate multiple invalid handle values such as NULL
//     and INVALID_HANDLE_VALUE (-1) for Win32 handles.
//   - Set() (and the constructors and assignment operators that call it)
//     preserve the Windows LastError code. This ensures that GetLastError() can
//     be called after stashing a handle in a GenericScopedHandle object. Doing
//     this explicitly is necessary because of bug 528394 and VC++ 2015.
template <class Traits, class Verifier>
class GenericScopedHandle {
 public:
  using Handle = typename Traits::Handle;

  GenericScopedHandle(const GenericScopedHandle&) = delete;
  GenericScopedHandle& operator=(const GenericScopedHandle&) = delete;

  GenericScopedHandle() : handle_(Traits::NullHandle()) {}

  explicit GenericScopedHandle(Handle handle) : handle_(Traits::NullHandle()) {
    Set(handle);
  }

  GenericScopedHandle(GenericScopedHandle&& other)
      : handle_(Traits::NullHandle()) {
    Set(other.Take());
  }

  ~GenericScopedHandle() { Close(); }

  bool IsValid() const { return Traits::IsHandleValid(handle_); }

  GenericScopedHandle& operator=(GenericScopedHandle&& other) {
    CR_DCHECK(this != &other);
    Set(other.Take());
    return *this;
  }

  void Set(Handle handle) {
    if (handle_ != handle) {
      // Preserve old LastError to avoid bug 528394.
      auto last_error = ::GetLastError();
      Close();

      if (Traits::IsHandleValid(handle)) {
        handle_ = handle;
        Verifier::StartTracking(handle, this, CRBASE_WIN_GET_CALLER,
                                GetProgramCounter());
      }
      ::SetLastError(last_error);
    }
  }

  Handle Get() const { return handle_; }

  // Transfers ownership away from this object.
  Handle Take() CR_WARN_UNUSED_RESULT {
    Handle temp = handle_;
    handle_ = Traits::NullHandle();
    if (Traits::IsHandleValid(temp)) {
      Verifier::StopTracking(temp, this, CRBASE_WIN_GET_CALLER,
                             GetProgramCounter());
    }
    return temp;
  }

  // Explicitly closes the owned handle.
  void Close() {
    if (Traits::IsHandleValid(handle_)) {
      Verifier::StopTracking(handle_, this, CRBASE_WIN_GET_CALLER,
                             GetProgramCounter());

      Traits::CloseHandle(handle_);
      handle_ = Traits::NullHandle();
    }
  }

 private:
  Handle handle_;
};

#undef CRBASE_WIN_GET_CALLER

// The traits class for Win32 handles that can be closed via CloseHandle() API.
class HandleTraits {
 public:
  using Handle = void*;

  HandleTraits() = delete;
  HandleTraits(const HandleTraits&) = delete;
  HandleTraits& operator=(const HandleTraits&) = delete;

  // Closes the handle.
  static bool CRBASE_EXPORT CloseHandle(Handle handle);

  // Returns true if the handle value is valid.
  static bool IsHandleValid(Handle handle) {
    return handle != nullptr && handle != INVALID_HANDLE_VALUE;
  }

  // Returns NULL handle value.
  static Handle NullHandle() { return nullptr; }
};

// Do-nothing verifier.
class DummyVerifierTraits {
 public:
  using Handle = void*;

  DummyVerifierTraits() = delete;
  DummyVerifierTraits(const DummyVerifierTraits&) = delete;
  DummyVerifierTraits& operator=(const DummyVerifierTraits&) = delete;

  static void StartTracking(Handle handle,
                            const void* owner,
                            const void* pc1,
                            const void* pc2) {}
  static void StopTracking(Handle handle,
                           const void* owner,
                           const void* pc1,
                           const void* pc2) {}
};

using ScopedHandle = GenericScopedHandle<HandleTraits, DummyVerifierTraits>;

}  // namespace win
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_SCOPED_HANDLE_H_
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_WINNT_HOOKER_H_
#define MINI_CHROMIUM_SRC_WINNT_HOOKER_H_

#include <stdint.h>
#include <stddef.h>

#include "cr_base/memory/singleton.h"
#include "cr_build/build_config.h"

#include "cr_winnt/winnt_export.h"

namespace cr {
namespace winnt {

class CRWINNT_EXPORT Hooker {
 public:
  Hooker(const Hooker&) = delete;
  Hooker& operator=(const Hooker&) = delete;

  static Hooker* GetInstance();

  //
  // target:   An address to hook.
  // detour:   An address to hook to.
  // original: To call the original code.
  //
  bool CreateHook(void* target, void* detour, void** original);

  // Warning: don not motify this structure!!
  struct CRWINNT_EXPORT ConnextRegister {
#if defined(MINI_CHROMIUM_ARCH_CPU_X86)
    uintptr_t eax;
    uintptr_t ebx;
    uintptr_t ecx;
    uintptr_t edx;
    uintptr_t edi;
    uintptr_t esi;
    uintptr_t ebp;
    uintptr_t esp;
#elif defined(MINI_CHROMIUM_ARCH_CPU_X86_64)
    uintptr_t r8;
    uintptr_t r9;
    uintptr_t r10;
    uintptr_t r11;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
    uintptr_t rax;
    uintptr_t rbx;
    uintptr_t rcx;
    uintptr_t rdx;
    uintptr_t rdi;
    uintptr_t rsi;
    uintptr_t rbp;
    uintptr_t rsp;
#endif
  };

  typedef void (* ContextCallback)(ConnextRegister*);
  bool CreateContextHook(void* target, ContextCallback callback);

  // A checker for avoid hook handler recursive calling.
  // for eaxmaple:
  //   FARPROC g_original;
  //   void WINAPI HookHandle() {
  //      ScopedAvoidRecursionFlag recursion_flag;
  //      if (recursion_flag->IsSet())
  //         return g_original();
  //      ....
  //      HookHandle();
  //   }
  struct CRWINNT_EXPORT ScopedAvoidRecursionFlag {
   public:
    ScopedAvoidRecursionFlag();
    ~ScopedAvoidRecursionFlag();

    bool IsSet() const { return flag_; };

   private:
    bool flag_;
  };

 private:
  friend struct cr::DefaultSingletonTraits<Hooker>;

  Hooker();
  ~Hooker();

  unsigned long tls_bypass_hook_;
};

}  // namespace winnt
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_WINNT_HOOKER_H_

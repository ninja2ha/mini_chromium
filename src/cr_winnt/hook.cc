// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_winnt/hook.h"

#include "cr_base/logging/logging.h"
#include "cr_base/memory/singleton.h"
#include "cr_base/memory/no_destructor.h"
#include "cr_base/threading/thread_local.h"

#include "cr_winnt/third_party/minhook/src/minhook.h"

namespace cr {
namespace winnt {

// static
Hooker* Hooker::GetInstance() {
  return Singleton<Hooker>::get();
}

Hooker::Hooker() {
  MH_STATUS status = MH_Initialize();

  CR_DLOG_IF(Error, status != MH_OK) 
      << "nhook initialize with an error:" << status;
}

Hooker::~Hooker() {
  MH_STATUS status = MH_Uninitialize();

  CR_DLOG_IF(Error, status != MH_OK) 
      << "nhook uninitialize with an error:" << status;
}

bool Hooker::CreateHook(void* target, void* detour, void** original) {
  MH_STATUS status = MH_CreateHook(target, detour, original);
  CR_DLOG_IF(Error, status != MH_OK) 
      << "nhook create hook with an error:" << status;
  return status == MH_OK;
}

bool Hooker::CreateContextHook(void* target, ContextCallback callback) {
#if defined(MINI_CHROMIUM_ARCH_CPU_X86)
  static_assert(offsetof(ConnextRegister, esp) == 7 * 4, 
                "dont motify the structure!");

  unsigned char shell_code[] = {
    0x54,       // push esp - 0x00
    0x55,       // push ebp - 0x01
    0x56,       // push esi - 0x02
    0x57,       // push edi - 0x03
    0x52,       // push edx - 0x04
    0x51,       // push ecx - 0x05
    0x53,       // push ebx - 0x06
    0x50,       // push eax - 0x07
    0x8B, 0xC4, // mov eax, esp  - 0x08
    0x50,       // push eax - 0x09
    0xB8, 0x00, 0x00, 0x00, 0x00, // mov eax, ... // 0x0A
    0xFF, 0xD0, // call eax - 0xF
    0x58,       // pop eax  - 0x11
    0x5B,       // pop ebx  - 0x12
    0x59,       // pop ecx  - 0x13
    0x5A,       // pop edx  - 0x14
    0x5F,       // pop edi  - 0x15
    0x5E,       // pop esi  - 0x16
    0x5D,       // pop ebp  - 0x17
    0x5C        // pop esp  - 0x18
  }; 

  *(uintptr_t*)(shell_code + 0x0A + 0x1) = (uintptr_t)callback;
#elif defined(MINI_CHROMIUM_ARCH_CPU_X86_64)
  static_assert(offsetof(ConnextRegister, rsp) == 15 * 8, 
                "dont motify the structure!");

  unsigned char shell_code[] = {
    0x54,       // push rsp - 0x00
    0x55,       // push rbp - 0x01
    0x56,       // push rsi - 0x02
    0x57,       // push rdi - 0x03
    0x52,       // push rdx - 0x04
    0x51,       // push rcx - 0x05
    0x53,       // push rbx - 0x06
    0x50,       // push rax - 0x07
    0x41, 0x57, // push r15 - 0x08
    0x41, 0x56, // push r14 - 0x0A
    0x41, 0x55, // push r13 - 0x0C
    0x41, 0x54, // push r12 - 0x0E
    0x41, 0x53, // push r11 - 0x10
    0x41, 0x52, // push r10 - 0x12
    0x41, 0x51, // push r9  - 0x14
    0x41, 0x50, // push r8  - 0x16
    0x48, 0x8B, 0xC4, // mov rax, rsp - 0x18
    0x50,       // push rax - 0x1B
    0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov eax, ... // 0x1C
    0xFF, 0xD0, // call eax - 0x26
    0x41, 0x58, // pop r8   - 0x28
    0x41, 0x59, // pop r9   - 0x2A
    0x41, 0x5A, // pop r10  - 0x2C
    0x41, 0x5B, // pop r11  - 0x2E
    0x41, 0x5C, // pop r12  - 0x30
    0x41, 0x5D, // pop r13  - 0x32
    0x41, 0x5E, // pop r14  - 0x34
    0x41, 0x5F, // pop r15  - 0x36
    0x58,       // pop eax  - 0x38
    0x5B,       // pop ebx  - 0x39 
    0x59,       // pop ecx  - 0x3A
    0x5A,       // pop edx  - 0x3B
    0x5F,       // pop edi  - 0x3C
    0x5E,       // pop esi  - 0x3D
    0x5D,       // pop ebp  - 0x3E
    0x5C        // pop esp  - 0x3F
  };

  *(uintptr_t*)(shell_code + 0x1C + 0x2) = (uintptr_t)callback;
#endif
  MH_STATUS status = 
      MH_CreateHookWithShell(target, NULL, shell_code, sizeof(shell_code));
  CR_DLOG_IF(Error, status != MH_OK) 
      << "nhook create context hook with an error:" << status;
  return status == MH_OK;
}

////////////////////////////////////////////////////////////////////////////////

namespace {

cr::ThreadLocalBoolean* GetAvoidRecursionTls() {
  static cr::NoDestructor<cr::ThreadLocalBoolean> tls;
  return tls.get();
}

}  // namespace

Hooker::ScopedAvoidRecursionFlag::ScopedAvoidRecursionFlag() {
  flag_ = GetAvoidRecursionTls()->Get();
  if (!flag_) {
    GetAvoidRecursionTls()->Set(true);
  }
}

Hooker::ScopedAvoidRecursionFlag::~ScopedAvoidRecursionFlag() {
  if (!flag_) {
    GetAvoidRecursionTls()->Set(false);
  }
}

}  // namespace winnt
}  // namespace cr
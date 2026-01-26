// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_DEBUG_IMMEDIATE_CRASH_H_
#define MINI_CHROMIUM_SRC_CRBASE_DEBUG_IMMEDIATE_CRASH_H_

#include "crbuild/build_config.h"

////////////////////////////////////////////////////////////////////////////////

// Crashes in the fastest possible way with no attempt at logging.
// There are several constraints; see http://crbug.com/664209 for more context.
//
// - CR_TRAP_SEQUENCE_() must be fatal. It should not be possible to ignore the
//   resulting exception or simply hit 'continue' to skip over it in a debugger.
// - Different instances of CR_TRAP_SEQUENCE_() must not be folded together, to
//   ensure crash reports are debuggable. Unlike __builtin_trap(), asm volatile
//   blocks will not be folded together.
//   Note: CR_TRAP_SEQUENCE_() previously required an instruction with a unique
//   nonce since unlike clang, GCC folds together identical asm volatile
//   blocks.
// - CR_TRAP_SEQUENCE_() must produce a signal that is distinct from an invalid
//   memory access.
// - CR_TRAP_SEQUENCE_() must be treated as a set of noreturn instructions.
//   __builtin_unreachable() is used to provide that hint here. clang also uses
//   this as a heuristic to pack the instructions in the function epilogue to
//   improve code density.
//
// Additional properties that are nice to have:
// - CR_TRAP_SEQUENCE_() should be as compact as possible.
// - The first instruction of TRAP_SEQUENCE_() should not change, to avoid
//   shifting crash reporting clusters. As a consequence of this, explicit
//   assembly is preferred over intrinsics.
//   Note: this last bullet point may no longer be true, and may be removed in
//   the future.

// Note: CR_TRAP_SEQUENCE Is currently split into two macro helpers due to the
// fact that clang emits an actual instruction for __builtin_unreachable() on
// certain platforms (see https://crbug.com/958675). In addition, the
// int3/bkpt/brk will be removed in followups, so splitting it up like this now
// makes it easy to land the followups.

#if defined(MINI_CHROMIUM_COMPILER_GCC)

#if defined(MINI_CHROMIUM_ARCH_CPU_X86_FAMILY)

// TODO(https://crbug.com/958675): In theory, it should be possible to use just
// int3. However, there are a number of crashes with SIGILL as the exception
// code, so it seems likely that there's a signal handler that allows execution
// to continue after SIGTRAP.
#define CR_TRAP_SEQUENCE1_() asm volatile("int3")

#if defined(MINI_CHROMIUM_OS_APPLE)
// Intentionally empty: __builtin_unreachable() is always part of the sequence
// (see IMMEDIATE_CRASH below) and already emits a ud2 on Mac.
#define CR_TRAP_SEQUENCE2_() asm volatile("")
#else
#define CR_TRAP_SEQUENCE2_() asm volatile("ud2")
#endif  // defined(MINI_CHROMIUM_OS_APPLE)

#elif defined(MINI_CHROMIUM_ARCH_CPU_ARMEL)

// bkpt will generate a SIGBUS when running on armv7 and a SIGTRAP when running
// as a 32 bit userspace app on arm64. There doesn't seem to be any way to
// cause a SIGTRAP from userspace without using a syscall (which would be a
// problem for sandboxing).
// TODO(https://crbug.com/958675): Remove bkpt from this sequence.
#define CR_TRAP_SEQUENCE1_() asm volatile("bkpt #0")
#define CR_TRAP_SEQUENCE2_() asm volatile("udf #0")

#elif defined(MINI_CHROMIUM_ARCH_CPU_ARM64)

// This will always generate a SIGTRAP on arm64.
// TODO(https://crbug.com/958675): Remove brk from this sequence.
#define CR_TRAP_SEQUENCE1_() asm volatile("brk #0")
#define CR_TRAP_SEQUENCE2_() asm volatile("hlt #0")

#else

// Crash report accuracy will not be guaranteed on other architectures, but at
// least this will crash as expected.
#define CR_TRAP_SEQUENCE1_() __builtin_trap()
#define CR_TRAP_SEQUENCE2_() asm volatile("")

#endif  // MINI_CHROMIUM_ARCH_CPU_*

#elif defined(MINI_CHROMIUM_COMPILER_MSVC)

#if !defined(__clang__)

// MSVC x64 doesn't support inline asm, so use the MSVC intrinsic.
#define CR_TRAP_SEQUENCE1_() __debugbreak()
#define CR_TRAP_SEQUENCE2_()

#elif defined(MINI_CHROMIUM_ARCH_CPU_ARM64)

// Windows ARM64 uses "BRK #F000" as its breakpoint instruction, and
// __debugbreak() generates that in both VC++ and clang.
#define CR_TRAP_SEQUENCE1_() __debugbreak()
// Intentionally empty: __builtin_unreachable() is always part of the sequence
// (see CR_IMMEDIATE_CRASH below) and already emits a ud2 on Win64,
// https://crbug.com/958373
#define CR_TRAP_SEQUENCE2_() __asm volatile("")

#else

#define CR_TRAP_SEQUENCE1_() asm volatile("int3")
#define CR_TRAP_SEQUENCE2_() asm volatile("ud2")

#endif  // __clang__

#else

#error No supported trap sequence!

#endif  // MINI_CHROMIUM_COMPILER_GCC

////////////////////////////////////////////////////////////////////////////////

#define CR_TRAP_SEQUENCE_() \
  do {                      \
    CR_TRAP_SEQUENCE1_();   \
    CR_TRAP_SEQUENCE2_();   \
  } while (false)

////////////////////////////////////////////////////////////////////////////////

// This version of CR_ALWAYS_INLINE inlines even in is_debug=true.
// TODO(pbos): See if NDEBUG can be dropped from CR_ALWAYS_INLINE as well,
// and if so merge. Otherwise document why it cannot inline in debug in
// crbase/compiler/compiler_specific.h.
#if defined(MINI_CHROMIUM_COMPILER_GCC)
#define IMMEDIATE_CRASH_ALWAYS_INLINE inline __attribute__((__always_inline__))
#elif defined(MINI_CHROMIUM_COMPILER_MSVC)
#if (_MSC_VER >= 1200)
#define IMMEDIATE_CRASH_ALWAYS_INLINE __forceinline
#else
#define IMMEDIATE_CRASH_ALWAYS_INLINE inline
#endif
#else
#define IMMEDIATE_CRASH_ALWAYS_INLINE inline
#endif

namespace cr {
namespace debug {

IMMEDIATE_CRASH_ALWAYS_INLINE void ImmediateCrash() {
  CR_TRAP_SEQUENCE_();
#if defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
  __builtin_unreachable();
#endif  // defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
}

void ImmediateCrashBecauseOutofMemory(size_t size);

}  // namespace debug
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_DEBUG_IMMEDIATE_CRASH_H_
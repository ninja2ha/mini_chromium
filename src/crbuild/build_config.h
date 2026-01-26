// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBUILD_BUILD_CONFIG_H_
#define MINI_CHROMIUM_SRC_CRBUILD_BUILD_CONFIG_H_

// This file adds defines about the platform we're currently building on.
//  Operating System:
//    MINI_CHROMIUM_OS_WIN
//    MINI_CHROMIUM_OS_LINUX
//    MINI_CHROMIUM_OS_OPENBSD
//    MINI_CHROMIUM_OS_FREEBSD
//    MINI_CHROMIUM_OS_NETBSD
//    MINI_CHROMIUM_OS_POSIX: LINUX or OPENBSD or FREEBSD or NETBSD
//    MINI_CHROMIUM_OS_BSD: OPENBSD or FREEBSD or NETBSD
//  Compiler:
//    MINI_CHROMIUM_COMPILER_MSVC 
//    MINI_CHROMIUM_COMPILER_GCC
//  Processor:
//    MINI_CHROMIUM_ARCH_CPU_X86 
//    MINI_CHROMIUM_ARCH_CPU_X86_64
//    MINI_CHROMIUM_ARCH_CPU_ARMEL 
//    MINI_CHROMIUM_ARCH_CPU_ARM64
//    MINI_CHROMIUM_ARCH_CPU_LOONGARCH32
//    MINI_CHROMIUM_ARCH_CPU_LOONGARCH364
//    MINI_CHROMIUM_ARCH_CPU_MIPS
//    MINI_CHROMIUM_ARCH_CPU_MIPS64
//    MINI_CHROMIUM_ARCH_CPU_MIPSEL
//    MINI_CHROMIUM_ARCH_CPU_MIPS64EL
//    MINI_CHROMIUM_ARCH_CPU_RISCV64
//  Processor family:
//    MINI_CHROMIUM_ARCH_CPU_X86_FAMILY: X86 or X86_64
//    MINI_CHROMIUM_ARCH_CPU_ARM_FAMILY: ARMEL or ARM64
//    MINI_CHROMIUM_ARCH_CPU_LOONGARCH_FAMILY: LOONGARCH32 or LOONGARCH64
//    MINI_CHROMIUM_ARCH_CPU_MIPS_FAMILY: MIPS64EL or MIPSEL or MIPS64 or MIPS
//    MINI_CHROMIUM_ARCH_CPU_RISCV_FAMILY: Riscv64
//  Processor features:
//    MINI_CHROMIUM_ARCH_CPU_32_BITS 
//    MINI_CHROMIUM_ARCH_CPU_64_BITS
//    MINI_CHROMIUM_ARCH_CPU_BIG_ENDIAN 
//    MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN

// A set of macros to use for platform detection.
#if defined(_WIN32) || defined(_WIN64)
#define MINI_CHROMIUM_OS_WIN 1
#elif defined(__linux__)
#define MINI_CHROMIUM_OS_LINUX 1
#elif defined(__FreeBSD__)
#define MINI_CHROMIUM_OS_FREEBSD 1
#elif defined(__NetBSD__)
#define MINI_CHROMIUM_OS_NETBSD 1
#elif defined(__OpenBSD__)
#define MINI_CHROMIUM_OS_OPENBSD 1
#else
#error Please add support for your platform  in cr/build_config.h
#endif

#if defined(MINI_CHROMIUM_OS_LINUX)  || defined(MINI_CHROMIUM_OS_FREEBSD) || \
    defined(MINI_CHROMIUM_OS_NETBSD) || defined(MINI_CHROMIUM_OS_OPENBSD)
#define MINI_CHROMIUM_OS_POSIX 1
#endif

// compiler
#if defined(__GNUC__)
#define MINI_CHROMIUM_COMPILER_GCC 1
#elif defined(_MSC_VER)
#define MINI_CHROMIUM_COMPILER_MSVC 1
#else
#error Please add support for your compipler in cr/build_config.h
#endif

// arch
#if defined(_M_X64) || defined(__x86_64__)
#define MINI_CHROMIUM_ARCH_CPU_X86_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_X86_64 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(__i386__)
#define MINI_CHROMIUM_ARCH_CPU_X86_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_X86 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__ARMEL__)
#define MINI_CHROMIUM_ARCH_CPU_ARM_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_ARMEL 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1

#elif defined(__aarch64__) || defined(_M_ARM64)
#define MINI_CHROMIUM_ARCH_CPU_ARM_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_ARM64 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1

#elif defined(__MIPSEL__)
#if defined(__LP64__)
#define MINI_CHROMIUM_ARCH_CPU_MIPS_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_MIPS64EL 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#else  // !defined(__LP64__)
#define MINI_CHROMIUM_ARCH_CPU_MIPS_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_MIPSEL 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#endif  // defined(__LP64__)

#elif defined(__MIPSEB__)
#if defined(__LP64__)
#define MINI_CHROMIUM_ARCH_CPU_MIPS_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_MIPS64 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_BIG_ENDIAN 1
#else  // !defined(__LP64__)
#define MINI_CHROMIUM_ARCH_CPU_MIPS_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_MIPS 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_BIG_ENDIAN 1
#endif  // defined(__LP64__)

#elif defined(__loongarch__)
#define MINI_CHROMIUM_ARCH_CPU_LOONGARCH_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#if __loongarch_grlen == 64
#define MINI_CHROMIUM_ARCH_CPU_LOONGARCH64 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#else
#define MINI_CHROMIUM_ARCH_CPU_LOONGARCH32 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#endif

#elif defined(__riscv) && (__riscv_xlen == 64)
#define MINI_CHROMIUM_ARCH_CPU_RISCV_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_RISCV64 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#else

#error Please add support for your architecture in cr/build_config.h
#endif

// Architecture-specific feature detection.

#if !defined(MINI_CHROMIUM_CPU_ARM_NEON)
#if defined(MINI_CHROMIUM_ARCH_CPU_ARM_FAMILY) && \
    (defined(__ARM_NEON__) || defined(__ARM_NEON))
#define MINI_CHROMIUM_CPU_ARM_NEON 1
#endif
#endif  // !defined(MINI_CHROMIUM_CPU_ARM_NEON)

// Sanity check.
#if defined(MINI_CHROMIUM_ARCH_CPU_ARM64) && \
    !defined(MINI_CHROMIUM_CPU_ARM_NEON)
#error "AArch64 mandates NEON, should be detected"
#endif

#if !defined(MINI_CHROMIUM_ARCH_CPU_MIPS_MSA)
#if defined(__mips_msa) && defined(__mips_isa_rev) && (__mips_isa_rev >= 5)
#define MINI_CHROMIUM_ARCH_CPU_MIPS_MSA 1
#endif
#endif

// Type detection for wchar_t.
#if defined(MINI_CHROMIUM_OS_WIN)
#define MINI_CHROMIUM_WCHAR_T_IS_UTF16
#elif defined(MINI_CHROMIUM_OS_POSIX) && \
      defined(MINI_CHROMIUM_COMPILER_GCC) && \
      defined(__WCHAR_MAX__) &&                                   \
      (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#define MINI_CHROMIUM_WCHAR_T_IS_UTF32
#elif defined(MINI_CHROMIUM_OS_POSIX) && defined(MINI_CHROMIUM_COMPILER_GCC) && \
      defined(__WCHAR_MAX__) &&                                                 \
      (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
// On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
// compile in this mode (in particular, Chrome doesn't). This is intended for
// other projects using base who manage their own dependencies and make sure
// short wchar works for them.
#define MINI_CHROMIUM_WCHAR_T_IS_UTF16
#else
#error Please add support for your compiler in cr/build_config.h
#endif

// file: BUILD.gn
// # Allow more direct string conversions on platforms with native utf8
// # strings
// if (is_apple || is_chromeos || is_castos || is_cast_android || is_fuchsia) {
//   defines += ["SYSTEM_NATIVE_UTF8"]
// }
#if defined(MINI_CHROMIUM_OS_APPLE)
#define MINI_CHROMIUM_SYSTEM_NATIVE_UTF8
#endif

#endif  // MINI_CHROMIUM_SRC_CRBUILD_BUILD_CONFIG_H_
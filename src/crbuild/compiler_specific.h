// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBUILD_COMPILER_SPECIFIC_H_
#define MINI_CHROMIUM_SRC_CRBUILD_COMPILER_SPECIFIC_H_

#include "crbuild/build_config.h"

// Annotate a function indicating it should not be inlined.
// Use like:
//   CR_NOINLINE void DoStuff() { ... }
#if defined(MINI_CHROMIUM_COMPILER_GCC)
#define CR_NOINLINE __attribute__((noinline))
#elif defined(MINI_CHROMIUM_COMPILER_MSVC)
#define CR_NOINLINE __declspec(noinline)
#else
#error Please add support for your compipler in cr/compiler_specific.h
#endif

#if defined(NDEBUG)
#if defined(MINI_CHROMIUM_COMPILER_GCC)
#define CR_ALWAYS_INLINE inline __attribute__((__always_inline__))
#elif defined(MINI_CHROMIUM_COMPILER_MSVC)
#if (_MSC_VER >= 1200)
#define CR_ALWAYS_INLINE __forceinline
#else
#define CR_ALWAYS_INLINE inline
#endif
#else
#error Please add support for your compipler in cr/compiler_specific.h
#endif
#else
#define CR_ALWAYS_INLINE
#endif

// Specify memory alignment for structs, classes, etc.
// Use like:
//   class CR_ALIGNAS(16) MyClass { ... }
//   CR_ALIGNAS(16) int array[4];
//
// In most places you can use the C++11 keyword "alignas", which is preferred.
//
// But compilers have trouble mixing __attribute__((...)) syntax with
// alignas(...) syntax.
//
// Doesn't work in clang or gcc:
//   struct alignas(16) __attribute__((packed)) S { char c; };
// Works in clang but not gcc:
//   struct __attribute__((packed)) alignas(16) S2 { char c; };
// Works in clang and gcc:
//   struct alignas(16) S3 { char c; } __attribute__((packed));
//
// There are also some attributes that must be specified *before* a class
// definition: visibility (used for exporting functions/classes) is one of
// these attributes. This means that it is not possible to use alignas() with a
// class that is marked as exported.
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#define CR_ALIGNAS(byte_alignment) __declspec(align(byte_alignment))
#elif defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
#define CR_ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#error Please add support for your compipler in cr/compiler_specific.h
#endif

// Annotate a function indicating the caller must examine the return value.
// Use like:
//   int foo() CR_WARN_UNUSED_RESULT;
// To explicitly ignore a result, see |ignore_result()| in base/macros.h.
#if defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
#define CR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define CR_WARN_UNUSED_RESULT
#endif

// Tell the compiler a function is using a printf-style format string.
// |format_param| is the one-based index of the format string parameter;
// |dots_param| is the one-based index of the "..." parameter.
// For v*printf functions (which take a va_list), pass 0 for dots_param.
// (This is undocumented but matches what the system C headers do.)
// For member functions, the implicit this parameter counts as index 1.
#if defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
#define CR_PRINTF_FORMAT(format_param, dots_param) \
  __attribute__((format(printf, format_param, dots_param)))
#else
#define CR_PRINTF_FORMAT(format_param, dots_param)
#endif

// CR_WPRINTF_FORMAT is the same, but for wide format strings.
// This doesn't appear to yet be implemented in any compiler.
// See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=38308 .
#define CR_WPRINTF_FORMAT(format_param, dots_param)
// If available, it would look like:
//   __attribute__((format(wprintf, format_param, dots_param)))

// Macro for hinting that an expression is likely to be false.
#if defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
#define CR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CR_UNLIKELY(x) (x)
#endif  // defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)

#if defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
#define CR_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define CR_LIKELY(x) (x)
#endif  // defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)

// Compiler feature-detection.
// clang.llvm.org/docs/LanguageExtensions.html#has-feature-and-has-extension
#if defined(__has_feature)
#define CR_HAS_FEATURE(FEATURE) __has_feature(FEATURE)
#else
#define CR_HAS_FEATURE(FEATURE) 0
#endif

// Macro for telling -Wimplicit-fallthrough that a fallthrough is intentional.
#if defined(__clang__)
#define CR_FALLTHROUGH [[clang::fallthrough]]
#else
#define CR_FALLTHROUGH
#endif

#endif  // MINI_CHROMIUM_SRC_CRBUILD_COMPILER_SPECIFIC_H_
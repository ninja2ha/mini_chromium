// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_TEST_GTEST_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_TEST_GTEST_UTIL_H_

#include <string>
#include <utility>
#include <vector>

#include "crbase/logging/logging.h"
#include "crbuild/compiler_specific.h"
#include "crbuild/build_config.h"

#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"

// EXPECT/ASSERT_DCHECK_DEATH is intended to replace EXPECT/ASSERT_DEBUG_DEATH
// when the death is expected to be caused by a DCHECK. Contrary to
// EXPECT/ASSERT_DEBUG_DEATH however, it doesn't execute the statement in non-
// dcheck builds as DCHECKs are intended to catch things that should never
// happen and as such executing the statement results in undefined behavior
// (|statement| is compiled in unsupported configurations nonetheless).
// Death tests misbehave on Android.
#if CR_DCHECK_IS_ON() && defined(GTEST_HAS_DEATH_TEST)

// EXPECT/ASSERT_DCHECK_DEATH tests verify that a DCHECK is hit ("Check failed"
// is part of the error message). Optionally you may specify part of the message
// to verify which DCHECK (or LOG(DFATAL)) is being hit.
#define EXPECT_DCHECK_DEATH(statement) EXPECT_DEATH(statement, "Check failed")
#define EXPECT_DCHECK_DEATH_WITH(statement, msg) EXPECT_DEATH(statement, msg)
#define ASSERT_DCHECK_DEATH(statement) ASSERT_DEATH(statement, "Check failed")
#define ASSERT_DCHECK_DEATH_WITH(statement, msg) ASSERT_DEATH(statement, msg)

#else
// DCHECK_IS_ON() && defined(GTEST_HAS_DEATH_TEST) && !defined(OS_ANDROID)

#define EXPECT_DCHECK_DEATH(statement) \
  GTEST_UNSUPPORTED_DEATH_TEST(statement, "Check failed", )
#define EXPECT_DCHECK_DEATH_WITH(statement, msg) \
  GTEST_UNSUPPORTED_DEATH_TEST(statement, msg, )
#define ASSERT_DCHECK_DEATH(statement) \
  GTEST_UNSUPPORTED_DEATH_TEST(statement, "Check failed", return )
#define ASSERT_DCHECK_DEATH_WITH(statement, msg) \
  GTEST_UNSUPPORTED_DEATH_TEST(statement, msg, return )

#endif
// DCHECK_IS_ON() && defined(GTEST_HAS_DEATH_TEST) && !defined(OS_ANDROID)

// As above, but for CHECK().
#if defined(GTEST_HAS_DEATH_TEST)

#if defined(NDEBUG)
#define EXPECT_CHECK_DEATH(statement) EXPECT_DEATH(statement, "")
#define ASSERT_CHECK_DEATH(statement) ASSERT_DEATH(statement, "")
#else
#define EXPECT_CHECK_DEATH(statement) EXPECT_DEATH(statement, "Check failed")
#define ASSERT_CHECK_DEATH(statement) ASSERT_DEATH(statement, "Check failed")
#endif

#else  // defined(GTEST_HAS_DEATH_TEST)

// Note GTEST_UNSUPPORTED_DEATH_TEST takes a |regex| only to see whether it is a
// valid regex. It is never evaluated.
#define EXPECT_CHECK_DEATH(statement) \
  GTEST_UNSUPPORTED_DEATH_TEST(statement, "", )
#define ASSERT_CHECK_DEATH(statement) \
  GTEST_UNSUPPORTED_DEATH_TEST(statement, "", return )

#endif  // defined(GTEST_HAS_DEATH_TEST)

#endif  // MINI_CHROMIUM_SRC_CRBASE_TEST_GTEST_UTIL_H_

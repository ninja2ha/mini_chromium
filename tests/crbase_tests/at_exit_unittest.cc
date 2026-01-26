// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UNIT_TEST
#define UNIT_TEST
#endif  // UNIT_TEST

#include "crbase/at_exit.h"
#include "crbase/functional/bind.h"

#include "gtest/gtest.h"

namespace {

int g_test_counter_1 = 0;
int g_test_counter_2 = 0;

void IncrementTestCounter1(void* unused) {
  ++g_test_counter_1;
}

void IncrementTestCounter2(void* unused) {
  ++g_test_counter_2;
}

void ZeroTestCounters() {
  g_test_counter_1 = 0;
  g_test_counter_2 = 0;
}

void ExpectCounter1IsZero(void* unused) {
  EXPECT_EQ(0, g_test_counter_1);
}

void ExpectParamIsNull(void* param) {
  EXPECT_EQ(nullptr, param);
}

void ExpectParamIsCounter(void* param) {
  EXPECT_EQ(&g_test_counter_1, param);
}

class ShadowingAtExitManager : public cr::AtExitManager {
 public:
  ShadowingAtExitManager() : cr::AtExitManager(true) {}
};

}  // namespace

class AtExitTest : public testing::Test {
 private:
  // Don't test the global AtExitManager, because asking it to process its
  // AtExit callbacks can ruin the global state that other tests may depend on.
  ShadowingAtExitManager exit_manager_;
};

TEST_F(AtExitTest, Basic) {
  ZeroTestCounters();
  cr::AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);
  cr::AtExitManager::RegisterCallback(&IncrementTestCounter2, nullptr);
  cr::AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);

  EXPECT_EQ(0, g_test_counter_1);
  EXPECT_EQ(0, g_test_counter_2);
  cr::AtExitManager::ProcessCallbacksNow();
  EXPECT_EQ(2, g_test_counter_1);
  EXPECT_EQ(1, g_test_counter_2);
}

TEST_F(AtExitTest, LIFOOrder) {
  ZeroTestCounters();
  cr::AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);
  cr::AtExitManager::RegisterCallback(&ExpectCounter1IsZero, nullptr);
  cr::AtExitManager::RegisterCallback(&IncrementTestCounter2, nullptr);

  EXPECT_EQ(0, g_test_counter_1);
  EXPECT_EQ(0, g_test_counter_2);
  cr::AtExitManager::ProcessCallbacksNow();
  EXPECT_EQ(1, g_test_counter_1);
  EXPECT_EQ(1, g_test_counter_2);
}

TEST_F(AtExitTest, Param) {
  cr::AtExitManager::RegisterCallback(&ExpectParamIsNull, nullptr);
  cr::AtExitManager::RegisterCallback(&ExpectParamIsCounter,
                                        &g_test_counter_1);
  cr::AtExitManager::ProcessCallbacksNow();
}

TEST_F(AtExitTest, Task) {
  ZeroTestCounters();
  cr::AtExitManager::RegisterTask(
      cr::BindOnce(&ExpectParamIsCounter, &g_test_counter_1));
  cr::AtExitManager::ProcessCallbacksNow();
}
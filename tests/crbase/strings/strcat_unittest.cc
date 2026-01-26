// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/strings/strcat.h"
#include "crbase/strings/utf_string_conversions.h"

#include "gtest/gtest.h"

namespace cr {

TEST(StrCat, char) {
  EXPECT_EQ("", StrCat({""}));
  EXPECT_EQ("1", StrCat({"1"}));
  EXPECT_EQ("122", StrCat({"1", "22"}));
  EXPECT_EQ("122333", StrCat({"1", "22", "333"}));
  EXPECT_EQ("1223334444", StrCat({"1", "22", "333", "4444"}));
  EXPECT_EQ("122333444455555", StrCat({"1", "22", "333", "4444", "55555"}));
}

TEST(StrCat, char16_t) {
  std::u16string arg1 = u"1";
  std::u16string arg2 = u"22";
  std::u16string arg3 = u"333";

  EXPECT_EQ(u"", StrCat({std::u16string()}));
  EXPECT_EQ(u"1", StrCat({arg1}));
  EXPECT_EQ(u"122", StrCat({arg1, arg2}));
  EXPECT_EQ(u"122333", StrCat({arg1, arg2, arg3}));
}

TEST(StrCat, wchar_t) {
  std::wstring arg1 = L"1";
  std::wstring arg2 = L"22";
  std::wstring arg3 = L"333";

  EXPECT_EQ(L"", StrCat({ std::wstring() }));
  EXPECT_EQ(L"1", StrCat({ arg1 }));
  EXPECT_EQ(L"122", StrCat({ arg1, arg2 }));
  EXPECT_EQ(L"122333", StrCat({ arg1, arg2, arg3 }));
}

TEST(StrAppend, char) {
  std::string result;

  result = "foo";
  StrAppend(&result, {std::string()});
  EXPECT_EQ("foo", result);

  result = "foo";
  StrAppend(&result, {"1"});
  EXPECT_EQ("foo1", result);

  result = "foo";
  StrAppend(&result, {"1", "22", "333"});
  EXPECT_EQ("foo122333", result);
}

TEST(StrAppend, char16_t) {
  std::u16string arg1 = u"1";
  std::u16string arg2 = u"22";
  std::u16string arg3 = u"333";

  std::u16string result;

  result = u"foo";
  StrAppend(&result, {std::u16string()});
  EXPECT_EQ(u"foo", result);

  result = u"foo";
  StrAppend(&result, {arg1});
  EXPECT_EQ(u"foo1", result);

  result = u"foo";
  StrAppend(&result, {arg1, arg2, arg3});
  EXPECT_EQ(u"foo122333", result);
}

TEST(StrAppend, wchar_t) {
  std::wstring arg1 = L"1";
  std::wstring arg2 = L"22";
  std::wstring arg3 = L"333";

  std::wstring result;

  result = L"foo";
  StrAppend(&result, {std::wstring()});
  EXPECT_EQ(L"foo", result);

  result = L"foo";
  StrAppend(&result, {arg1});
  EXPECT_EQ(L"foo1", result);

  result = L"foo";
  StrAppend(&result, {arg1, arg2, arg3});
  EXPECT_EQ(L"foo122333", result);
}

TEST(StrAppendT, ReserveAdditionalIfNeeded) {
  std::string str = "foo";
  const char* prev_data = str.data();
  size_t prev_capacity = str.capacity();
  // Fully exhaust current capacity.
  StrAppend(&str, {std::string(str.capacity() - str.size(), 'o')});
  // Expect that we hit capacity, but didn't require a re-alloc.
  EXPECT_EQ(str.capacity(), str.size());
  EXPECT_EQ(prev_data, str.data());
  EXPECT_EQ(prev_capacity, str.capacity());

  // Force a re-alloc by appending another character.
  StrAppend(&str, {"o"});

  // Expect at least 2x growth in capacity.
  EXPECT_LE(2 * prev_capacity, str.capacity());
}

}  // namespace cr
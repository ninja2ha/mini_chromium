// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_INTERNAL_GTEST_PROD_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_INTERNAL_GTEST_PROD_UTIL_H_

// When you need to test the private or protected members of a class,
// use the FRIEND_TEST macro to declare your tests as friends of the
// class.  For example:
//
// class MyClass {
//  private:
//   void PrivateMethod();
//   CR_FRIEND_GTEST(MyClassTest, PrivateMethodWorks);
// };
//
// class MyClassTest : public testing::Test {
//   // ...
// };
//
// TEST_F(MyClassTest, PrivateMethodWorks) {
//   // Can call MyClass::PrivateMethod() here.
// }
//
// Note: The test class must be in the same namespace as the class being tested.
// For example, putting MyClassTest in an anonymous namespace will not work.

#define CR_FRIEND_GTEST(test_case_name, test_name) \
  friend class test_case_name##_##test_name##_Test


// This is a wrapper for gtest's FRIEND_TEST macro that friends
// test with all possible prefixes. This is very helpful when changing the test
// prefix, because the friend declarations don't need to be updated.
//
// Example usage:
//
// class MyClass {
//  private:
//   void MyMethod();
//   CR_FRIEND_TEST_ALL_PREFIXES(MyClassTest, MyMethod);
// };
#define CR_FRIEND_GTEST_ALL_PREFIXES(test_case_name, test_name) \
  CR_FRIEND_GTEST(test_case_name, test_name); \
  CR_FRIEND_GTEST(test_case_name, DISABLED_##test_name); \
  CR_FRIEND_GTEST(test_case_name, FLAKY_##test_name)

// C++ compilers will refuse to compile the following code:
//
// namespace foo {
// class MyClass {
//  private:
//   CR_FRIEND_GTEST_ALL_PREFIXES(MyClassTest, TestMethod);
//   bool private_var;
// };
// }  // namespace foo
//
// class MyClassTest::TestMethod() {
//   foo::MyClass foo_class;
//   foo_class.private_var = true;
// }
//
// Unless you forward declare MyClassTest::TestMethod outside of namespace foo.
// Use FORWARD_DECLARE_TEST to do so for all possible prefixes.
//
// Example usage:
//
// CR_FORWARD_DECLARE_GTEST(MyClassTest, TestMethod);
//
// namespace foo {
// class MyClass {
//  private:
//   CR_FRIEND_TEST_ALL_PREFIXES(::MyClassTest, TestMethod);  // NOTE use of ::
//   bool private_var;
// };
// }  // namespace foo
//
// class MyClassTest::TestMethod() {
//   foo::MyClass foo_class;
//   foo_class.private_var = true;
// }

#define CR_FORWARD_DECLARE_GTEST(test_case_name, test_name) \
  class test_case_name##_##test_name##_Test; \
  class test_case_name##_##DISABLED_##test_name##_Test; \
  class test_case_name##_##FLAKY_##test_name##_Test

#endif  // MINI_CHROMIUM_SRC_CRBASE_INTERNAL_GTEST_PROD_UTIL_H_
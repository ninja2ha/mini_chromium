// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/synchronization/lock.h"

#include <stdlib.h>

#include <thread>
#include <memory>

#include "crbuild/compiler_specific.h"

#include "gtest/gtest.h"

#include "crbase/gtest_util.h"

namespace cr {

// Basic test to make sure that Acquire()/Release()/Try() don't crash ----------

class BasicLockTestThread {
 public:
  BasicLockTestThread(const BasicLockTestThread&) = delete;
  BasicLockTestThread& operator=(const BasicLockTestThread&) = delete;

  explicit BasicLockTestThread(Lock* lock) : lock_(lock), acquired_(0) {}

  void ThreadMain() {
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      lock_->Acquire();
      acquired_++;
      PlatformThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
      lock_->Release();
    }
    for (int i = 0; i < 10; i++) {
      if (lock_->Try()) {
        acquired_++;
        PlatformThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
        lock_->Release();
      }
    }
  }

  bool Create() {
    if (!thread_) {
      thread_ = std::make_unique<std::thread>(
          &BasicLockTestThread::ThreadMain, this);
    }
    return thread_ ? thread_->joinable() : false;
  }

  void Join() {
    if (thread_)
      thread_->join();
  }

  int acquired() const { return acquired_; }

 private:
  Lock* lock_;
  int acquired_;
  std::unique_ptr<std::thread> thread_;
};

TEST(LockTest, Basic) {
  Lock lock;
  BasicLockTestThread thread(&lock);

  ASSERT_TRUE(thread.Create());

  int acquired = 0;
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    lock.Acquire();
    acquired++;
    PlatformThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
    lock.Release();
  }
  for (int i = 0; i < 10; i++) {
    if (lock.Try()) {
      acquired++;
      PlatformThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
      lock.Release();
    }
  }
  for (int i = 0; i < 5; i++) {
    lock.Acquire();
    acquired++;
    PlatformThread::Sleep(TimeDelta::FromMilliseconds(rand() % 20));
    lock.Release();
  }

  thread.Join();

  EXPECT_GE(acquired, 20);
  EXPECT_GE(thread.acquired(), 20);
}

// Test that Try() works as expected -------------------------------------------

class TryLockTestThread {
 public:
  TryLockTestThread(const TryLockTestThread&) = delete;
  TryLockTestThread& operator=(const TryLockTestThread&) = delete;

  explicit TryLockTestThread(Lock* lock) : lock_(lock), got_lock_(false) {}

  void ThreadMain() {
    // The local variable is required for the static analyzer to see that the
    // lock is properly released.
    bool got_lock = lock_->Try();
    got_lock_ = got_lock;
    if (got_lock)
      lock_->Release();
  }

  bool Create() {
    if (!thread_) {
      thread_ = std::make_unique<std::thread>(
          &TryLockTestThread::ThreadMain, this);
    }
    return thread_ ? thread_->joinable() : false;
  }

  void Join() {
    if (thread_)
      thread_->join();
  }

  bool got_lock() const { return got_lock_; }

 private:
  Lock* lock_;
  bool got_lock_;
  std::unique_ptr<std::thread> thread_;
};

TEST(LockTest, TryLock) {
  Lock lock;

  ASSERT_TRUE(lock.Try());
  lock.AssertAcquired();

  // This thread will not be able to get the lock.
  {
    TryLockTestThread thread(&lock);

    ASSERT_TRUE(thread.Create());

    thread.Join();

    ASSERT_FALSE(thread.got_lock());
  }

  lock.Release();

  // This thread will....
  {
    TryLockTestThread thread(&lock);

    ASSERT_TRUE(thread.Create());

    thread.Join();

    ASSERT_TRUE(thread.got_lock());
    // But it released it....
    ASSERT_TRUE(lock.Try());
    lock.AssertAcquired();
  }

  lock.Release();
}

TEST(LockTest, TryTrackedLock) {
  Lock lock;

  ASSERT_TRUE(lock.Try());
  lock.AssertAcquired();

  // This thread will not be able to get the lock.
  {
    TryLockTestThread thread(&lock);

    ASSERT_TRUE(thread.Create());

    thread.Join();

    ASSERT_FALSE(thread.got_lock());
  }

  lock.Release();

  // This thread will....
  {
    TryLockTestThread thread(&lock);

    ASSERT_TRUE(thread.Create());

    thread.Join();

    ASSERT_TRUE(thread.got_lock());
    // But it released it....
    ASSERT_TRUE(lock.Try());
    lock.AssertAcquired();
  }

  lock.Release();
}

// Tests that locks actually exclude -------------------------------------------

class MutexLockTestThread {
 public:
  MutexLockTestThread(const MutexLockTestThread&) = delete;
  MutexLockTestThread& operator=(const MutexLockTestThread&) = delete;

  MutexLockTestThread(Lock* lock, int* value) : lock_(lock), value_(value) {}

  // Static helper which can also be called from the main thread.
  static void DoStuff(Lock* lock, int* value) {
    for (int i = 0; i < 40; i++) {
      lock->Acquire();
      int v = *value;
      PlatformThread::Sleep(TimeDelta::FromMilliseconds(rand() % 10));
      *value = v + 1;
      lock->Release();
    }
  }

  void ThreadMain() { DoStuff(lock_, value_); }

  bool Create() {
    if (!thread_) {
      thread_ = std::make_unique<std::thread>(
          &MutexLockTestThread::ThreadMain, this);
    }
    return thread_ ? thread_->joinable() : false;
  }

  void Join() {
    if (thread_)
      thread_->join();
  }

 private:
  Lock* lock_;
  int* value_;
  std::unique_ptr<std::thread> thread_;
};

TEST(LockTest, MutexTwoThreads) {
  Lock lock;
  int value = 0;

  MutexLockTestThread thread(&lock, &value);

  ASSERT_TRUE(thread.Create());

  MutexLockTestThread::DoStuff(&lock, &value);

  thread.Join();

  EXPECT_EQ(2 * 40, value);
}

TEST(LockTest, MutexFourThreads) {
  Lock lock;
  int value = 0;

  MutexLockTestThread thread1(&lock, &value);
  MutexLockTestThread thread2(&lock, &value);
  MutexLockTestThread thread3(&lock, &value);

  ASSERT_TRUE(thread1.Create());
  ASSERT_TRUE(thread2.Create());
  ASSERT_TRUE(thread3.Create());

  MutexLockTestThread::DoStuff(&lock, &value);

  thread1.Join();
  thread2.Join();
  thread3.Join();

  EXPECT_EQ(4 * 40, value);
}

}  // namespace cr
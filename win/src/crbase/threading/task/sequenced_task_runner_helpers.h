// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_SEQUENCED_TASK_RUNNER_HELPERS_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_SEQUENCED_TASK_RUNNER_HELPERS_H_

namespace cr {

class SequencedTaskRunner;

// Template helpers which use function indirection to erase T from the
// function signature while still remembering it so we can call the
// correct destructor/release function.
//
// We use this trick so we don't need to include bind.h in a header
// file like sequenced_task_runner.h. We also wrap the helpers in a
// templated class to make it easier for users of DeleteSoon to
// declare the helper as a friend.
template <class T>
class DeleteHelper {
 private:
  static void DoDelete(const void* object) {
    delete static_cast<const T*>(object);
  }

  friend class SequencedTaskRunner;
};

template <class T>
class ReleaseHelper {
 private:
  static void DoRelease(const void* object) {
    static_cast<const T*>(object)->Release();
  }

  friend class SequencedTaskRunner;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_SEQUENCED_TASK_RUNNER_HELPERS_H_
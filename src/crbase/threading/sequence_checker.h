// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_SEQUENCE_CHECKER_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_SEQUENCE_CHECKER_H_

#include "crbase/logging/logging.h"
#include "crbase/strings/string_piece.h"
#include "crbase/threading/sequence_checker_impl.h"
#include "crbuild/compiler_specific.h"
#include "crbuild/build_config.h"

// SequenceChecker is a helper class used to help verify that some methods of a
// class are called sequentially (for thread-safety). It supports thread safety
// annotations (see base/thread_annotations.h).
//
// Use the macros below instead of the SequenceChecker directly so that the
// unused member doesn't result in an extra byte (four when padded) per
// instance in production.
//
// This class is much prefered to ThreadChecker for thread-safety checks.
// ThreadChecker should only be used for classes that are truly thread-affine
// (use thread-local-storage or a third-party API that does).
//
// Usage:
//   class MyClass {
//    public:
//     MyClass() {
//       // It's sometimes useful to detach on construction for objects that are
//       // constructed in one place and forever after used from another
//       // sequence.
//       CR_DETACH_FROM_SEQUENCE(my_sequence_checker_);
//     }
//
//     ~MyClass() {
//       // SequenceChecker doesn't automatically check it's destroyed on origin
//       // sequence for the same reason it's sometimes detached in the
//       // constructor. It's okay to destroy off sequence if the owner
//       // otherwise knows usage on the associated sequence is done. If you're
//       // not detaching in the constructor, you probably want to explicitly
//       // check in the destructor.
//       CR_DCHECK_CALLED_ON_VALID_SEQUENCE(my_sequence_checker_);
//     }
//     void MyMethod() {
//       CR_DCHECK_CALLED_ON_VALID_SEQUENCE(my_sequence_checker_);
//       ... (do stuff) ...
//       MyOtherMethod();
//     }
//
//     void MyOtherMethod() {
//       foo_ = 42;
//     }
//
//    private:
//     int foo_;
//
//     CR_SEQUENCE_CHECKER(my_sequence_checker_);
//   }

#if CR_DCHECK_IS_ON()
#define CR_SEQUENCE_CHECKER(name) cr::SequenceChecker name;
#define CR_DCHECK_CALLED_ON_VALID_SEQUENCE(name) \
  CR_DCHECK((name).CalledOnValidSequence())
#define CR_DETACH_FROM_SEQUENCE(name) (name).DetachFromSequence()
#else  // CR_DCHECK_IS_ON()
#define CR_SEQUENCE_CHECKER(name)
#define CR_DCHECK_CALLED_ON_VALID_SEQUENCE(name) CR_EAT_STREAM_PARAMETERS(true)
#define CR_DETACH_FROM_SEQUENCE(name)
#endif  // CR_DCHECK_IS_ON()

namespace cr {

// Do nothing implementation, for use in release mode.
//
// Note: You should almost always use the SequenceChecker class (through the
// above macros) to get the right version for your build configuration.
class SequenceCheckerDoNothing {
 public:
  SequenceCheckerDoNothing(
      const SequenceCheckerDoNothing&) = delete;
  SequenceCheckerDoNothing& operator=(
      const SequenceCheckerDoNothing&) = delete;
  SequenceCheckerDoNothing() = default;

  // Moving between matching sequences is allowed to help classes with
  // SequenceCheckers that want a default move-construct/assign.
  SequenceCheckerDoNothing(SequenceCheckerDoNothing&& other) = default;
  SequenceCheckerDoNothing& operator=(SequenceCheckerDoNothing&& other) =
      default;

  bool CalledOnValidSequence() const CR_WARN_UNUSED_RESULT { return true; }
  void DetachFromSequence() {}
};

#if CR_DCHECK_IS_ON()
class SequenceChecker : public SequenceCheckerImpl {
};
#else
class SequenceChecker : public SequenceCheckerDoNothing {
};
#endif  // DCHECK_IS_ON()

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_SAFE_CHECKER_SEQUENCE_CHECKER_H_
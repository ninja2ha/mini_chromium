// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_COMPLETION_REPEATING_CALLBACK_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_COMPLETION_REPEATING_CALLBACK_H_

#include <stdint.h>

#include "cr_base/functional/callback.h"
#include "cr_base/functional/cancelable_callback.h"

namespace crnet {

// A RepeatingCallback specialization that takes a single int parameter. Usually
// this is used to report a byte count or network error code.
using CompletionRepeatingCallback = cr::RepeatingCallback<void(int)>;

// 64bit version of the RepeatingCallback specialization that takes a single
// int64_t parameter. Usually this is used to report a file offset, size or
// network error code.
using Int64CompletionRepeatingCallback = cr::RepeatingCallback<void(int64_t)>;

using CancelableCompletionRepeatingCallback =
    cr::CancelableRepeatingCallback<void(int)>;

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_COMPLETION_REPEATING_CALLBACK_H_
// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_BASE_COMPLETION_ONCE_CALLBACK_H_
#define MINI_CHROMIUM_SRC_CRNET_BASE_COMPLETION_ONCE_CALLBACK_H_

#include <stdint.h>

#include "crbase/functional/callback.h"
#include "crbase/functional/cancelable_callback.h"

namespace crnet {

// A OnceCallback specialization that takes a single int parameter. Usually this
// is used to report a byte count or network error code.
using CompletionOnceCallback = cr::OnceCallback<void(int)>;

// 64bit version of the OnceCallback specialization that takes a single int64_t
// parameter. Usually this is used to report a file offset, size or network
// error code.
using Int64CompletionOnceCallback = cr::OnceCallback<void(int64_t)>;

using CancelableCompletionOnceCallback =
    cr::CancelableOnceCallback<void(int)>;

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_BASE_COMPLETION_ONCE_CALLBACK_H_
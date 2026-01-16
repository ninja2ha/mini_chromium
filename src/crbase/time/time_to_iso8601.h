// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_TIME_TIME_TO_ISO8601_H_
#define MINI_CHROMIUM_SRC_CRBASE_TIME_TIME_TO_ISO8601_H_

#include <string>

#include "crbase/base_export.h"

namespace cr {

class Time;

CRBASE_EXPORT std::string TimeToISO8601(const cr::Time& t);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_TIME_TIME_TO_ISO8601_H_

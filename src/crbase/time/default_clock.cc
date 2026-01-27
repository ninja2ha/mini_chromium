// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/time/default_clock.h"

#include "crbase/memory/no_destructor.h"

namespace cr {

DefaultClock::~DefaultClock() = default;

Time DefaultClock::Now() const {
  return Time::Now();
}

// static
DefaultClock* DefaultClock::GetInstance() {
  static cr::NoDestructor<DefaultClock> instance;
  return instance.get();
}

}  // namespace cr

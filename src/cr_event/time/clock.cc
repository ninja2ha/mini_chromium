// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_event/time/clock.h"

#include "cr_base/memory/no_destructor.h"

namespace cr {

namespace {
// DefaultClock is a Clock implementation that uses Time::Now().
class DefaultClock : public Clock {
 public:
  ~DefaultClock() override = default;

  // Simply returns Time::Now().
  Time Now() const override {
    return Time::Now();
  }
};
}  // namespace

Clock::~Clock() = default;

// static
Clock* Clock::GetDefault() {
  static cr::NoDestructor<DefaultClock> instance;
  return instance.get();
}

}  // namespace cr

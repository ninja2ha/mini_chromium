// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/sequence/sequence_token.h"

#include "crbase/atomic/atomic_sequence_num.h"
#include "crbase/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"

namespace cr {

namespace {

cr::AtomicSequenceNumber g_sequence_token_generator;

cr::AtomicSequenceNumber g_task_token_generator;

ThreadLocalPointer<const SequenceToken>& GetTlsCurrentSequenceToken() {
  static cr::NoDestructor<ThreadLocalPointer<const SequenceToken>> instance;
  return *instance;
}

ThreadLocalPointer<const TaskToken>& GetTlsCurrentTaskToken() {
  static cr::NoDestructor<ThreadLocalPointer<const TaskToken>> instance;
  return *instance;
}

}  // namespace

bool SequenceToken::operator==(const SequenceToken& other) const {
  return token_ == other.token_ && IsValid();
}

bool SequenceToken::operator!=(const SequenceToken& other) const {
  return !(*this == other);
}

bool SequenceToken::IsValid() const {
  return token_ != kInvalidSequenceToken;
}

int SequenceToken::ToInternalValue() const {
  return token_;
}

SequenceToken SequenceToken::Create() {
  return SequenceToken(g_sequence_token_generator.GetNext());
}

SequenceToken SequenceToken::GetForCurrentThread() {
  const SequenceToken* current_sequence_token =
      GetTlsCurrentSequenceToken().Get();
  return current_sequence_token ? *current_sequence_token : SequenceToken();
}

bool TaskToken::operator==(const TaskToken& other) const {
  return token_ == other.token_ && IsValid();
}

bool TaskToken::operator!=(const TaskToken& other) const {
  return !(*this == other);
}

bool TaskToken::IsValid() const {
  return token_ != kInvalidTaskToken;
}

TaskToken TaskToken::Create() {
  return TaskToken(g_task_token_generator.GetNext());
}

TaskToken TaskToken::GetForCurrentThread() {
  const TaskToken* current_task_token = GetTlsCurrentTaskToken().Get();
  return current_task_token ? *current_task_token : TaskToken();
}

}  // namespace cr
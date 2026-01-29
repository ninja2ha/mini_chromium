// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase_runtime/threading/sequence_local_storage_map.h"

#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"

namespace cr {
namespace internal {

namespace {

ThreadLocalPointer<SequenceLocalStorageMap>* TlsCurrentSequenceLocalStorage() {
  static cr::NoDestructor<ThreadLocalPointer<SequenceLocalStorageMap>> tls;
  return tls.get();
}

}  // namespace

SequenceLocalStorageMap::SequenceLocalStorageMap() = default;

SequenceLocalStorageMap::~SequenceLocalStorageMap() = default;

ScopedSetSequenceLocalStorageMapForCurrentThread::
    ScopedSetSequenceLocalStorageMapForCurrentThread(
        SequenceLocalStorageMap* sequence_local_storage) {
  CR_DCHECK(!TlsCurrentSequenceLocalStorage()->Get());
  TlsCurrentSequenceLocalStorage()->Set(sequence_local_storage);
}

ScopedSetSequenceLocalStorageMapForCurrentThread::
    ~ScopedSetSequenceLocalStorageMapForCurrentThread() {
  TlsCurrentSequenceLocalStorage()->Set(nullptr);
}

SequenceLocalStorageMap& SequenceLocalStorageMap::GetForCurrentThread() {
  SequenceLocalStorageMap* current_sequence_local_storage =
      TlsCurrentSequenceLocalStorage()->Get();

  CR_DCHECK(current_sequence_local_storage)
      << "SequenceLocalStorageSlot cannot be used because no "
         "SequenceLocalStorageMap was stored in TLS. Use "
         "ScopedSetSequenceLocalStorageMapForCurrentThread to store a "
         "SequenceLocalStorageMap object in TLS.";

  return *current_sequence_local_storage;
}

void* SequenceLocalStorageMap::Get(int slot_id) {
  const auto it = sls_map_.find(slot_id);
  if (it == sls_map_.end())
    return nullptr;
  return it->second.value();
}

void SequenceLocalStorageMap::Set(
    int slot_id,
    SequenceLocalStorageMap::ValueDestructorPair value_destructor_pair) {
  auto it = sls_map_.find(slot_id);

  if (it == sls_map_.end())
    sls_map_.emplace(slot_id, std::move(value_destructor_pair));
  else
    it->second = std::move(value_destructor_pair);

  // The maximum number of entries in the map is 256. This can be adjusted, but
  // will require reviewing the choice of data structure for the map.
  CR_DCHECK(sls_map_.size() <= 256U);
}

SequenceLocalStorageMap::ValueDestructorPair::ValueDestructorPair(
    void* value,
    DestructorFunc* destructor)
    : value_(value), destructor_(destructor) {}

SequenceLocalStorageMap::ValueDestructorPair::~ValueDestructorPair() {
  if (value_)
    destructor_(value_);
}

SequenceLocalStorageMap::ValueDestructorPair::ValueDestructorPair(
    ValueDestructorPair&& value_destructor_pair)
    : value_(value_destructor_pair.value_),
      destructor_(value_destructor_pair.destructor_) {
  value_destructor_pair.value_ = nullptr;
}

SequenceLocalStorageMap::ValueDestructorPair&
SequenceLocalStorageMap::ValueDestructorPair::operator=(
    ValueDestructorPair&& value_destructor_pair) {
  // Destroy |value_| before overwriting it with a new value.
  if (value_)
    destructor_(value_);

  value_ = value_destructor_pair.value_;
  destructor_ = value_destructor_pair.destructor_;

  value_destructor_pair.value_ = nullptr;

  return *this;
}

}  // namespace internal
}  // namespace cr
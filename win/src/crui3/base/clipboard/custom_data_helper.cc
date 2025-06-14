// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TODO(dcheng): For efficiency reasons, consider passing custom data around
// as a vector instead. It allows us to append a
// std::pair<base::string16, base::string16> and swap the deserialized values.

#include "crui/base/clipboard/custom_data_helper.h"

#include <utility>

#include "crbase/buffer/pickle.h"
#include "crbase/helper/stl_util.h"

namespace crui {

namespace {

class SkippablePickle : public cr::Pickle {
 public:
  SkippablePickle(const void* data, size_t data_len);
  bool SkipString16(cr::PickleIterator* iter);
};

SkippablePickle::SkippablePickle(const void* data, size_t data_len)
    : cr::Pickle(reinterpret_cast<const char*>(data), data_len) {
}

bool SkippablePickle::SkipString16(cr::PickleIterator* iter) {
  CR_DCHECK(iter);

  size_t len;
  if (!iter->ReadLength(&len))
    return false;
  return iter->SkipBytes(static_cast<int>(len * sizeof(cr::char16)));
}

}  // namespace

void ReadCustomDataTypes(const void* data,
                         size_t data_length,
                         std::vector<cr::string16>* types) {
  SkippablePickle pickle(data, data_length);
  cr::PickleIterator iter(pickle);

  uint32_t size = 0;
  if (!iter.ReadUInt32(&size))
    return;

  // Keep track of the original elements in the types vector. On failure, we
  // truncate the vector to the original size since we want to ignore corrupt
  // custom data pickles.
  size_t original_size = types->size();

  for (uint32_t i = 0; i < size; ++i) {
    types->push_back(cr::string16());
    if (!iter.ReadString16(&types->back()) || !pickle.SkipString16(&iter)) {
      types->resize(original_size);
      return;
    }
  }
}

void ReadCustomDataForType(const void* data,
                           size_t data_length,
                           const cr::string16& type,
                           cr::string16* result) {
  SkippablePickle pickle(data, data_length);
  cr::PickleIterator iter(pickle);

  uint32_t size = 0;
  if (!iter.ReadUInt32(&size))
    return;

  for (uint32_t i = 0; i < size; ++i) {
    cr::string16 deserialized_type;
    if (!iter.ReadString16(&deserialized_type))
      return;
    if (deserialized_type == type) {
      cr::ignore_result(iter.ReadString16(result));
      return;
    }
    if (!pickle.SkipString16(&iter))
      return;
  }
}

void ReadCustomDataIntoMap(
    const void* data,
    size_t data_length,
    std::unordered_map<cr::string16, cr::string16>* result) {
  cr::Pickle pickle(reinterpret_cast<const char*>(data), data_length);
  cr::PickleIterator iter(pickle);

  uint32_t size = 0;
  if (!iter.ReadUInt32(&size))
    return;

  for (uint32_t i = 0; i < size; ++i) {
    cr::string16 type;
    if (!iter.ReadString16(&type)) {
      // Data is corrupt, return an empty map.
      result->clear();
      return;
    }
    auto insert_result = result->insert({type, cr::string16()});
    if (!iter.ReadString16(&insert_result.first->second)) {
      // Data is corrupt, return an empty map.
      result->clear();
      return;
    }
  }
}

void WriteCustomDataToPickle(
    const std::unordered_map<cr::string16, cr::string16>& data,
    cr::Pickle* pickle) {
  pickle->WriteUInt32(static_cast<uint32_t>(data.size()));
  for (const auto& it : data) {
    pickle->WriteString16(it.first);
    pickle->WriteString16(it.second);
  }
}

void WriteCustomDataToPickle(
    const cr::flat_map<cr::string16, cr::string16>& data,
    cr::Pickle* pickle) {
  pickle->WriteUInt32(static_cast<uint32_t>(data.size()));
  for (const auto& it : data) {
    pickle->WriteString16(it.first);
    pickle->WriteString16(it.second);
  }
}

}  // namespace crui

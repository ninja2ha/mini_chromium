// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_METADATA_PROPERTY_METADATA_H_
#define UI_VIEWS_METADATA_PROPERTY_METADATA_H_

#include <string>
#include <type_traits>

#include "crbase/strings/string16.h"
#include "crui/views/metadata/metadata_cache.h"
#include "crui/views/metadata/type_conversion.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {
namespace metadata {

// Represents meta data for a specific read-only property member of class
// |TClass|, with underlying type |TValue|, as the type of the actual member.
// Using a separate |TRet| type for the getter function's return type to allow
// it to return a type with qualifier and by reference.
template <typename TClass,
          typename TValue,
          typename TRet,
          TRet (TClass::*Get)() const>
class ClassPropertyReadOnlyMetaData : public MemberMetaDataBase {
 public:
  ClassPropertyReadOnlyMetaData(
      const ClassPropertyReadOnlyMetaData&) = delete;
  ClassPropertyReadOnlyMetaData& operator=(
      const ClassPropertyReadOnlyMetaData&) = delete;

  ClassPropertyReadOnlyMetaData() = default;
  ~ClassPropertyReadOnlyMetaData() override = default;

  cr::string16 GetValueAsString(void* obj) const override {
    return TypeConverter<TValue>::ToString((static_cast<TClass*>(obj)->*Get)());
  }

  PropertyFlags GetPropertyFlags() const override {
    return PropertyFlags::kReadOnly;
  }
};

// Represents meta data for a specific property member of class |TClass|, with
// underlying type |TValue|, as the type of the actual member.
// Allows for interaction with the property as if it were the underlying data
// type (|TValue|), but still uses the Property's functionality under the hood
// (so it will trigger things like property changed notifications).
template <typename TClass,
          typename TValue,
          void (TClass::*Set)(ArgType<TValue>),
          typename TRet,
          TRet (TClass::*Get)() const>
class ClassPropertyMetaData
    : public ClassPropertyReadOnlyMetaData<TClass, TValue, TRet, Get> {
 public:
  ClassPropertyMetaData(
      const ClassPropertyMetaData&) = delete;
  ClassPropertyMetaData& operator=(
      const ClassPropertyMetaData&) = delete;

  ClassPropertyMetaData() = default;
  ~ClassPropertyMetaData() override = default;

  void SetValueAsString(void* obj, const cr::string16& new_value) override {
    if (cr::Optional<TValue> result =
            TypeConverter<TValue>::FromString(new_value))
      (static_cast<TClass*>(obj)->*Set)(result.value());
  }

  PropertyFlags GetPropertyFlags() const override {
    return PropertyFlags::kEmpty;
  }
};

}  // namespace metadata
}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_METADATA_PROPERTY_METADATA_H_

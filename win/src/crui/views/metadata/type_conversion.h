// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_METADATA_TYPE_CONVERSION_H_
#define UI_VIEWS_METADATA_TYPE_CONVERSION_H_

#include <stdint.h>
#include <vector>

#include "crbase/memory/no_destructor.h"
#include "crbase/containers/optional.h"
#include "crbase/strings/string16.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/strings/stringprintf.h"
#include "crbase/strings/sys_string_conversions.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/time/time.h"
#include "crui/gfx/geometry/size.h"
#include "crui/gfx/geometry/range.h"
///#include "crui/gfx/shadow_value.h"
#include "crui/gfx/text_constants.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {
namespace metadata {

// Various metadata methods pass types either by value or const ref depending on
// whether the types are "small" (defined as "fundamental, enum, or pointer").
// ArgType<T> gives the appropriate type to use as an argument in such cases.
template <typename T>
using ArgType = typename std::conditional<std::is_fundamental<T>::value ||
                                              std::is_enum<T>::value ||
                                              std::is_pointer<T>::value,
                                          T,
                                          const T&>::type;

// General Type Conversion Template Functions ---------------------------------
template <typename T>
struct TypeConverter {
  static cr::string16 ToString(ArgType<T> source_value);
  static cr::Optional<T> FromString(const cr::string16& source_value);
};

// Types and macros for generating enum converters ----------------------------
template <typename T>
struct EnumStrings {
  struct EnumString {
    T enum_value;
    cr::string16 str_value;
  };

  explicit EnumStrings(std::vector<EnumString> init_val)
      : pairs(std::move(init_val)) {}

  const std::vector<EnumString> pairs;
};

template <typename T>
static const EnumStrings<T>& GetEnumStringsInstance();

// Generate the code to define a enum type to and from base::string16
// conversions. The first argument is the type T, and the rest of the argument
// should have the enum value and string pairs defined in a format like
// "{enum_value0, string16_value0}, {enum_value1, string16_value1} ...".
#define DEFINE_ENUM_CONVERTERS(T, ...)                                         \
  template <>                                                                  \
  const crui::views::metadata::EnumStrings<T>&                                 \
  crui::views::metadata::GetEnumStringsInstance<T>() {                         \
    static const cr::NoDestructor<EnumStrings<T>> instance(                    \
        std::vector<crui::views::metadata::EnumStrings<T>::EnumString>(        \
            {__VA_ARGS__}));                                                   \
    return *instance;                                                          \
  }                                                                            \
                                                                               \
  template <>                                                                  \
  cr::string16 crui::views::metadata::TypeConverter<T>::ToString(              \
      ArgType<T> source_value) {                                               \
    for (const auto& pair : crui::views::metadata::GetEnumStringsInstance<T>() \
                                .pairs) {                                      \
      if (source_value == pair.enum_value)                                     \
        return pair.str_value;                                                 \
    }                                                                          \
    return cr::string16();                                                     \
  }                                                                            \
                                                                               \
  template <>                                                                  \
  cr::Optional<T> crui::views::metadata::TypeConverter<T>::FromString(         \
      const cr::string16& source_value) {                                      \
    for (const auto& pair : crui::views::metadata::GetEnumStringsInstance<T>() \
                                .pairs) {                                      \
      if (source_value == pair.str_value) {                                    \
        return pair.enum_value;                                                \
      }                                                                        \
    }                                                                          \
    return cr::nullopt;                                                        \
  }

// String Conversions ---------------------------------------------------------

#define DECLARE_CONVERSIONS(T)                                               \
  template <>                                                                \
  struct CRUI_EXPORT TypeConverter<T> {                                      \
    static cr::string16 ToString(ArgType<T> source_value);                   \
    static cr::Optional<T> FromString(const cr::string16& source_value);     \
  };

DECLARE_CONVERSIONS(int8_t)
DECLARE_CONVERSIONS(int16_t)
DECLARE_CONVERSIONS(int32_t)
DECLARE_CONVERSIONS(int64_t)
DECLARE_CONVERSIONS(uint8_t)
DECLARE_CONVERSIONS(uint16_t)
DECLARE_CONVERSIONS(uint32_t)
DECLARE_CONVERSIONS(uint64_t)
DECLARE_CONVERSIONS(float)
DECLARE_CONVERSIONS(double)
DECLARE_CONVERSIONS(bool)
DECLARE_CONVERSIONS(const char*)
DECLARE_CONVERSIONS(cr::string16)
DECLARE_CONVERSIONS(cr::TimeDelta)
///DECLARE_CONVERSIONS(gfx::ShadowValues)
DECLARE_CONVERSIONS(crui::gfx::Size)
DECLARE_CONVERSIONS(crui::gfx::Range)

#undef DECLARE_CONVERSIONS

// Special Conversions for cr::Optional<T> type --------------------------------

CRUI_EXPORT const cr::string16& GetNullOptStr();

template <typename T>
struct TypeConverter<cr::Optional<T>> {
  static cr::string16 ToString(ArgType<cr::Optional<T>> source_value) {
    if (!source_value)
      return GetNullOptStr();
    return TypeConverter<T>::ToString(source_value.value());
  }
  static cr::Optional<cr::Optional<T>> FromString(
      const cr::string16& source_value) {
    if (source_value == GetNullOptStr())
      return cr::make_optional<cr::Optional<T>>(cr::nullopt);

    auto ret = TypeConverter<T>::FromString(source_value);
    return ret ? cr::make_optional(ret) : cr::nullopt;
  }
};

}  // namespace metadata
}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_METADATA_TYPE_CONVERSION_H_

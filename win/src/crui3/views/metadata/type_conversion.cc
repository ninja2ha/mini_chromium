// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/metadata/type_conversion.h"

#include "crbase/strings/string16.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/strings/string_split.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/stringprintf.h"
#include "crbase/strings/sys_string_conversions.h"
#include "crbase/strings/utf_string_conversions.h"
///#include "crui/base/ime/text_input_type.h"
#include "crui/gfx/geometry/rect.h"

namespace crui {
namespace views {
namespace metadata {

const cr::string16& GetNullOptStr() {
  static const cr::NoDestructor<cr::string16> kNullOptStr(
      cr::ASCIIToUTF16("<Empty>"));
  return *kNullOptStr;
}

/***** String Conversions *****/

#define CONVERT_NUMBER_TO_STRING(T)                         \
  cr::string16 TypeConverter<T>::ToString(T source_value) { \
    return cr::NumberToString16(source_value);              \
  }

CONVERT_NUMBER_TO_STRING(int8_t)
CONVERT_NUMBER_TO_STRING(int16_t)
CONVERT_NUMBER_TO_STRING(int32_t)
CONVERT_NUMBER_TO_STRING(int64_t)
CONVERT_NUMBER_TO_STRING(uint8_t)
CONVERT_NUMBER_TO_STRING(uint16_t)
CONVERT_NUMBER_TO_STRING(uint32_t)
CONVERT_NUMBER_TO_STRING(uint64_t)
CONVERT_NUMBER_TO_STRING(float)
CONVERT_NUMBER_TO_STRING(double)

cr::string16 TypeConverter<bool>::ToString(bool source_value) {
  return cr::ASCIIToUTF16(source_value ? "true" : "false");
}

cr::string16 TypeConverter<const char*>::ToString(const char* source_value) {
  return cr::UTF8ToUTF16(source_value);
}

cr::string16 TypeConverter<cr::string16>::ToString(
    const cr::string16& source_value) {
  return source_value;
}

cr::string16 TypeConverter<cr::TimeDelta>::ToString(
    const cr::TimeDelta& source_value) {
  return cr::NumberToString16(source_value.InSecondsF()) +
         cr::ASCIIToUTF16(" s");
}

///cr::string16 TypeConverter<gfx::ShadowValues>::ToString(
///    const gfx::ShadowValues& source_value) {
///  cr::string16 ret = base::ASCIIToUTF16("[");
///  for (auto shadow_value : source_value) {
///    ret += cr::ASCIIToUTF16(" " + shadow_value.ToString() + ";");
///  }
///
///  ret[ret.length() - 1] = ' ';
///  ret += cr::ASCIIToUTF16("]");
///  return ret;
///}

cr::string16 TypeConverter<gfx::Size>::ToString(
    const gfx::Size& source_value) {
  return cr::ASCIIToUTF16(cr::StringPrintf("{%i, %i}", source_value.width(),
                                           source_value.height()));
}

cr::string16 TypeConverter<gfx::Range>::ToString(
    const gfx::Range& source_value) {
  return cr::ASCIIToUTF16(cr::StringPrintf(
      "{%i, %i}", source_value.GetMin(), source_value.GetMax()));
}

cr::Optional<int8_t> TypeConverter<int8_t>::FromString(
    const cr::string16& source_value) {
  int32_t ret = 0;
  if (cr::StringToInt(source_value, &ret) &&
      cr::IsValueInRangeForNumericType<int8_t>(ret)) {
    return static_cast<int8_t>(ret);
  }
  return cr::nullopt;
}

cr::Optional<int16_t> TypeConverter<int16_t>::FromString(
    const cr::string16& source_value) {
  int32_t ret = 0;
  if (cr::StringToInt(source_value, &ret) &&
      cr::IsValueInRangeForNumericType<int16_t>(ret)) {
    return static_cast<int16_t>(ret);
  }
  return cr::nullopt;
}

cr::Optional<int32_t> TypeConverter<int32_t>::FromString(
    const cr::string16& source_value) {
  int value;
  return cr::StringToInt(source_value, &value) ? cr::make_optional(value)
                                               : cr::nullopt;
}

cr::Optional<int64_t> TypeConverter<int64_t>::FromString(
    const cr::string16& source_value) {
  int64_t value;
  return cr::StringToInt64(source_value, &value) ? cr::make_optional(value)
                                                 : cr::nullopt;
}

cr::Optional<uint8_t> TypeConverter<uint8_t>::FromString(
    const cr::string16& source_value) {
  uint32_t ret = 0;
  if (cr::StringToUint(source_value, &ret) &&
      cr::IsValueInRangeForNumericType<uint8_t>(ret)) {
    return static_cast<uint8_t>(ret);
  }
  return cr::nullopt;
}

cr::Optional<uint16_t> TypeConverter<uint16_t>::FromString(
    const cr::string16& source_value) {
  uint32_t ret = 0;
  if (cr::StringToUint(source_value, &ret) &&
      cr::IsValueInRangeForNumericType<uint16_t>(ret)) {
    return static_cast<uint16_t>(ret);
  }
  return cr::nullopt;
}

cr::Optional<uint32_t> TypeConverter<uint32_t>::FromString(
    const cr::string16& source_value) {
  unsigned int value;
  return cr::StringToUint(source_value, &value) ? cr::make_optional(value)
                                                : cr::nullopt;
}

cr::Optional<uint64_t> TypeConverter<uint64_t>::FromString(
    const cr::string16& source_value) {
  uint64_t value;
  return cr::StringToUint64(source_value, &value) ? cr::make_optional(value)
                                                  : cr::nullopt;
}

cr::Optional<float> TypeConverter<float>::FromString(
    const cr::string16& source_value) {
  if (cr::Optional<double> temp =
          TypeConverter<double>::FromString(source_value))
    return static_cast<float>(temp.value());
  return cr::nullopt;
}

cr::Optional<double> TypeConverter<double>::FromString(
    const cr::string16& source_value) {
  double value;
  return cr::StringToDouble(cr::UTF16ToUTF8(source_value), &value)
             ? cr::make_optional(value)
             : cr::nullopt;
}

cr::Optional<bool> TypeConverter<bool>::FromString(
    const cr::string16& source_value) {
  const bool is_true = source_value == cr::ASCIIToUTF16("true");
  if (is_true || source_value == cr::ASCIIToUTF16("false"))
    return is_true;
  return cr::nullopt;
}

cr::Optional<cr::string16> TypeConverter<cr::string16>::FromString(
    const cr::string16& source_value) {
  return source_value;
}

cr::Optional<cr::TimeDelta> TypeConverter<cr::TimeDelta>::FromString(
    const cr::string16& source_value) {
  if (!cr::EndsWith(source_value, cr::ASCIIToUTF16(" s"),
                    cr::CompareCase::SENSITIVE))
    return cr::nullopt;
  double ret;
  return cr::StringToDouble(source_value.substr(0, source_value.length() - 2),
                              &ret)
             ? cr::make_optional(cr::TimeDelta::FromSecondsD(ret))
             : cr::nullopt;
}

///cr::Optional<gfx::ShadowValues> TypeConverter<gfx::ShadowValues>::FromString(
///    const cr::string16& source_value) {
///  gfx::ShadowValues ret;
///  const auto shadow_value_strings =
///      cr::SplitStringPiece(source_value, cr::ASCIIToUTF16("[;]"),
///                           cr::TRIM_WHITESPACE, cr::SPLIT_WANT_NONEMPTY);
///
///  for (auto v : shadow_value_strings) {
///    cr::string16 member_string;
///    cr::RemoveChars(v.as_string(), cr::ASCIIToUTF16("()rgba"),
///                    &member_string);
///    const auto members = cr::SplitStringPiece(
///        member_string, cr::ASCIIToUTF16(","), cr::TRIM_WHITESPACE,
///        cr::SPLIT_WANT_NONEMPTY);
///    int x, y, r, g, b, a;
///    double blur;
///
///    if ((members.size() == 7) && cr::StringToInt(members[0], &x) &&
///        cr::StringToInt(members[1], &y) &&
///        cr::StringToDouble(UTF16ToASCII(members[2]), &blur) &&
///        cr::StringToInt(members[3], &r) &&
///        cr::StringToInt(members[4], &g) &&
///        cr::StringToInt(members[5], &b) && cr::StringToInt(members[6], &a))
///      ret.emplace_back(gfx::Vector2d(x, y), blur, SkColorSetARGB(a, r, g, b));
///  }
///  return ret;
///}

cr::Optional<gfx::Size> TypeConverter<gfx::Size>::FromString(
    const cr::string16& source_value) {
  const auto values =
      cr::SplitStringPiece(source_value, cr::ASCIIToUTF16("{,}"),
                           cr::TRIM_WHITESPACE, cr::SPLIT_WANT_NONEMPTY);
  int width, height;
  if ((values.size() == 2) && cr::StringToInt(values[0], &width) &&
      cr::StringToInt(values[1], &height)) {
    return gfx::Size(width, height);
  }
  return cr::nullopt;
}

cr::Optional<gfx::Range> TypeConverter<gfx::Range>::FromString(
    const cr::string16& source_value) {
  const auto values =
    cr::SplitStringPiece(source_value, cr::ASCIIToUTF16("{,}"),
                         cr::TRIM_WHITESPACE, cr::SPLIT_WANT_NONEMPTY);
  int min, max;
  if ((values.size() == 2) && cr::StringToInt(values[0], &min) &&
      cr::StringToInt(values[1], &max)) {
    return gfx::Range(min, max);
  }
  return cr::nullopt;
}

}  // namespace metadata
}  // namespace views
}  // namespace crui

DEFINE_ENUM_CONVERTERS(crui::gfx::HorizontalAlignment,
                       {crui::gfx::HorizontalAlignment::ALIGN_LEFT,
                        cr::ASCIIToUTF16("ALIGN_LEFT")},
                       {crui::gfx::HorizontalAlignment::ALIGN_CENTER,
                        cr::ASCIIToUTF16("ALIGN_CENTER")},
                       {crui::gfx::HorizontalAlignment::ALIGN_RIGHT,
                        cr::ASCIIToUTF16("ALIGN_RIGHT")},
                       {crui::gfx::HorizontalAlignment::ALIGN_TO_HEAD,
                        cr::ASCIIToUTF16("ALIGN_TO_HEAD")})

DEFINE_ENUM_CONVERTERS(crui::gfx::VerticalAlignment,
                       {crui::gfx::VerticalAlignment::ALIGN_TOP, 
                        cr::ASCIIToUTF16("ALIGN_TOP")},
                       {crui::gfx::VerticalAlignment::ALIGN_MIDDLE, 
                        cr::ASCIIToUTF16("ALIGN_MIDDLE")},
                       {crui::gfx::VerticalAlignment::ALIGN_BOTTOM, 
                        cr::ASCIIToUTF16("ALIGN_BOTTOM")})

DEFINE_ENUM_CONVERTERS(
    crui::gfx::ElideBehavior,
    {crui::gfx::ElideBehavior::NO_ELIDE, cr::ASCIIToUTF16("NO_ELIDE")},
    {crui::gfx::ElideBehavior::TRUNCATE, cr::ASCIIToUTF16("TRUNCATE")},
    {crui::gfx::ElideBehavior::ELIDE_HEAD, cr::ASCIIToUTF16("ELIDE_HEAD")},
    {crui::gfx::ElideBehavior::ELIDE_MIDDLE, cr::ASCIIToUTF16("ELIDE_MIDDLE")},
    {crui::gfx::ElideBehavior::ELIDE_TAIL, cr::ASCIIToUTF16("ELIDE_TAIL")},
    {crui::gfx::ElideBehavior::ELIDE_EMAIL, cr::ASCIIToUTF16("ELIDE_EMAIL")},
    {crui::gfx::ElideBehavior::FADE_TAIL, cr::ASCIIToUTF16("FADE_TAIL")})

///DEFINE_ENUM_CONVERTERS(crui::TextInputType,
///                       {crui::TextInputType::TEXT_INPUT_TYPE_NONE,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_NONE")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_TEXT,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_TEXT")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_PASSWORD,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_PASSWORD")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_SEARCH,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_SEARCH")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_EMAIL,
///                        cr::ASCIIToUTF16("EXT_INPUT_TYPE_EMAIL")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_NUMBER,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_NUMBER")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_TELEPHONE,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_TELEPHONE")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_URL,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_URL")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_DATE,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_DATE")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_DATE_TIME,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_DATE_TIME")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_DATE_TIME_LOCAL,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_DATE_TIME_LOCAL")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_MONTH,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_MONTH")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_TIME,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_TIME")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_WEEK,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_WEEK")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_TEXT_AREA,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_TEXT_AREA")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_CONTENT_EDITABLE,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_CONTENT_EDITABLE")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_DATE_TIME_FIELD,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_DATE_TIME_FIELD")},
///                       {crui::TextInputType::TEXT_INPUT_TYPE_MAX,
///                        cr::ASCIIToUTF16("TEXT_INPUT_TYPE_MAX")})

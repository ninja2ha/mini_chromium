// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/values.h"

#include <string.h>

#include <algorithm>
#include <cmath>
#include <new>
#include <ostream>
#include <utility>

#include "cr_base/logging/logging.h"
#include "cr_base/bit_cast.h"
#include "cr_base/json/json_writer.h"
#include "cr_base/memory/ptr_util.h"
#include "cr_base/stl_util.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/strings/utf_string_conversions.h"
#include "cr_base/containers/internal/checked_iterators.h"

namespace cr {

namespace {

const char* const kTypeNames[] = {"null",   "boolean", "integer",    "double",
                                  "string", "binary",  "dictionary", "list"};
static_assert(cr::size(kTypeNames) ==
                  static_cast<size_t>(Value::Type::LIST) + 1,
              "kTypeNames Has Wrong Size");

std::unique_ptr<Value> CopyWithoutEmptyChildren(const Value& node);

// Make a deep copy of |node|, but don't include empty lists or dictionaries
// in the copy. It's possible for this function to return NULL and it
// expects |node| to always be non-NULL.
std::unique_ptr<Value> CopyListWithoutEmptyChildren(const Value& list) {
  Value copy(Value::Type::LIST);
  for (const auto& entry : list.GetList()) {
    std::unique_ptr<Value> child_copy = CopyWithoutEmptyChildren(entry);
    if (child_copy)
      copy.Append(std::move(*child_copy));
  }
  return copy.GetList().empty() ? nullptr
                                : std::make_unique<Value>(std::move(copy));
}

std::unique_ptr<DictionaryValue> CopyDictionaryWithoutEmptyChildren(
    const DictionaryValue& dict) {
  std::unique_ptr<DictionaryValue> copy;
  for (const auto& it : dict.DictItems()) {
    std::unique_ptr<Value> child_copy = CopyWithoutEmptyChildren(it.second);
    if (child_copy) {
      if (!copy)
        copy = std::make_unique<DictionaryValue>();
      copy->SetKey(it.first, std::move(*child_copy));
    }
  }
  return copy;
}

std::unique_ptr<Value> CopyWithoutEmptyChildren(const Value& node) {
  switch (node.type()) {
    case Value::Type::LIST:
      return CopyListWithoutEmptyChildren(static_cast<const ListValue&>(node));

    case Value::Type::DICTIONARY:
      return CopyDictionaryWithoutEmptyChildren(
          static_cast<const DictionaryValue&>(node));

    default:
      return std::make_unique<Value>(node.Clone());
  }
}

// Helper class to enumerate the path components from a StringPiece
// without performing heap allocations. Components are simply separated
// by single dots (e.g. "foo.bar.baz"  -> ["foo", "bar", "baz"]).
//
// Usage example:
//    PathSplitter splitter(some_path);
//    while (splitter.HasNext()) {
//       StringPiece component = splitter.Next();
//       ...
//    }
//
class PathSplitter {
 public:
  explicit PathSplitter(StringPiece path) : path_(path) {}

  bool HasNext() const { return pos_ < path_.size(); }

  StringPiece Next() {
    CR_DCHECK(HasNext());
    size_t start = pos_;
    size_t pos = path_.find('.', start);
    size_t end;
    if (pos == path_.npos) {
      end = path_.size();
      pos_ = end;
    } else {
      end = pos;
      pos_ = pos + 1;
    }
    return path_.substr(start, end - start);
  }

 private:
  StringPiece path_;
  size_t pos_ = 0;
};

}  // namespace

// static
Value Value::FromUniquePtrValue(std::unique_ptr<Value> val) {
  return std::move(*val);
}

// static
std::unique_ptr<Value> Value::ToUniquePtrValue(Value val) {
  return std::make_unique<Value>(std::move(val));
}

// static
const DictionaryValue& Value::AsDictionaryValue(const Value& val) {
  CR_CHECK(val.is_dict());
  return static_cast<const DictionaryValue&>(val);
}

// static
const ListValue& Value::AsListValue(const Value& val) {
  CR_CHECK(val.is_list());
  return static_cast<const ListValue&>(val);
}

Value::Value(Value&& that) noexcept {
  InternalMoveConstructFrom(std::move(that));
}

Value::Value(Type type) : type_(type) {
  // Initialize with the default value.
  switch (type_) {
    case Type::NONE:
      return;

    case Type::BOOLEAN:
      bool_value_ = false;
      return;
    case Type::INTEGER:
      int_value_ = 0;
      return;
    case Type::DOUBLE:
      double_value_ = BitCast<DoubleStorage>(0.0);
      return;
    case Type::STRING:
      new (&string_value_) std::string();
      return;
    case Type::BINARY:
      new (&binary_value_) BlobStorage();
      return;
    case Type::DICTIONARY:
      new (&dict_) DictStorage();
      return;
    case Type::LIST:
      new (&list_) ListStorage();
      return;
      // TODO(crbug.com/859477): Remove after root cause is found.
    case Type::DEAD:
      CR_CHECK(false);
      return;
  }

  // TODO(crbug.com/859477): Revert to NOTREACHED() after root cause is found.
  CR_CHECK(false);
}

Value::Value(bool in_bool) : type_(Type::BOOLEAN), bool_value_(in_bool) {}

Value::Value(int in_int) : type_(Type::INTEGER), int_value_(in_int) {}

Value::Value(double in_double)
    : type_(Type::DOUBLE), double_value_(BitCast<DoubleStorage>(in_double)) {
  if (!std::isfinite(in_double)) {
    CR_NOTREACHED() << "Non-finite (i.e. NaN or positive/negative infinity) "
                    << "values cannot be represented in JSON";
    double_value_ = BitCast<DoubleStorage>(0.0);
  }
}

Value::Value(const char* in_string) : Value(std::string(in_string)) {}

Value::Value(StringPiece in_string) : Value(std::string(in_string)) {}

Value::Value(std::string&& in_string) noexcept
    : type_(Type::STRING), string_value_(std::move(in_string)) {
  CR_DCHECK(IsStringUTF8AllowingNoncharacters(string_value_));
}

Value::Value(const char16_t* in_string16) : Value(StringPiece16(in_string16)) {}

Value::Value(StringPiece16 in_string16) : Value(UTF16ToUTF8(in_string16)) {}

Value::Value(const std::vector<char>& in_blob)
    : type_(Type::BINARY), binary_value_(in_blob.begin(), in_blob.end()) {}

Value::Value(cr::Span<const uint8_t> in_blob)
    : type_(Type::BINARY), binary_value_(in_blob.begin(), in_blob.end()) {}

Value::Value(BlobStorage&& in_blob) noexcept
    : type_(Type::BINARY), binary_value_(std::move(in_blob)) {}

Value::Value(const DictStorage& in_dict) : type_(Type::DICTIONARY), dict_() {
  dict_.reserve(in_dict.size());
  for (const auto& it : in_dict) {
    dict_.try_emplace(dict_.end(), it.first,
                      std::make_unique<Value>(it.second->Clone()));
  }
}

Value::Value(DictStorage&& in_dict) noexcept
    : type_(Type::DICTIONARY), dict_(std::move(in_dict)) {}

Value::Value(Span<const Value> in_list) : type_(Type::LIST), list_() {
  list_.reserve(in_list.size());
  for (const auto& val : in_list)
    list_.emplace_back(val.Clone());
}

Value::Value(ListStorage&& in_list) noexcept
    : type_(Type::LIST), list_(std::move(in_list)) {}

Value& Value::operator=(Value&& that) noexcept {
  InternalCleanup();
  InternalMoveConstructFrom(std::move(that));

  return *this;
}

double Value::AsDoubleInternal() const {
  return BitCast<double>(double_value_);
}

Value Value::Clone() const {
  switch (type_) {
    case Type::NONE:
      return Value();
    case Type::BOOLEAN:
      return Value(bool_value_);
    case Type::INTEGER:
      return Value(int_value_);
    case Type::DOUBLE:
      return Value(AsDoubleInternal());
    case Type::STRING:
      return Value(string_value_);
    case Type::BINARY:
      return Value(binary_value_);
    case Type::DICTIONARY:
      return Value(dict_);
    case Type::LIST:
      return Value(list_);
      // TODO(crbug.com/859477): Remove after root cause is found.
    case Type::DEAD:
      CR_CHECK(false);
      return Value();
  }

  // TODO(crbug.com/859477): Revert to NOTREACHED() after root cause is found.
  CR_CHECK(false);
  return Value();
}

Value::~Value() {
  InternalCleanup();
  // TODO(crbug.com/859477): Remove after root cause is found.
  type_ = Type::DEAD;
}

// static
const char* Value::GetTypeName(Value::Type type) {
  CR_DCHECK(static_cast<int>(type) >= 0);
  CR_DCHECK(static_cast<size_t>(type) < cr::size(kTypeNames));
  return kTypeNames[static_cast<size_t>(type)];
}

bool Value::GetBool() const {
  CR_CHECK(is_bool());
  return bool_value_;
}

int Value::GetInt() const {
  CR_CHECK(is_int());
  return int_value_;
}

double Value::GetDouble() const {
  if (is_double())
    return AsDoubleInternal();
  if (is_int())
    return int_value_;
  CR_CHECK(false);
  return 0.0;
}

const std::string& Value::GetString() const {
  CR_CHECK(is_string());
  return string_value_;
}

std::string& Value::GetString() {
  CR_CHECK(is_string());
  return string_value_;
}

const Value::BlobStorage& Value::GetBlob() const {
  CR_CHECK(is_blob());
  return binary_value_;
}

Value::ListView Value::GetList() {
  CR_CHECK(is_list());
  return list_;
}

Value::ConstListView Value::GetList() const {
  CR_CHECK(is_list());
  return list_;
}

Value::ListStorage Value::TakeList() {
  CR_CHECK(is_list());
  return std::exchange(list_, ListStorage());
}

void Value::Append(bool value) {
  CR_CHECK(is_list());
  list_.emplace_back(value);
}

void Value::Append(int value) {
  CR_CHECK(is_list());
  list_.emplace_back(value);
}

void Value::Append(double value) {
  CR_CHECK(is_list());
  list_.emplace_back(value);
}

void Value::Append(const char* value) {
  CR_CHECK(is_list());
  list_.emplace_back(value);
}

void Value::Append(StringPiece value) {
  CR_CHECK(is_list());
  list_.emplace_back(value);
}

void Value::Append(std::string&& value) {
  CR_CHECK(is_list());
  list_.emplace_back(std::move(value));
}

void Value::Append(const char16_t* value) {
  CR_CHECK(is_list());
  list_.emplace_back(value);
}

void Value::Append(StringPiece16 value) {
  CR_CHECK(is_list());
  list_.emplace_back(value);
}

void Value::Append(Value&& value) {
  CR_CHECK(is_list());
  list_.emplace_back(std::move(value));
}

internal::CheckedContiguousIterator<Value> Value::Insert(
    internal::CheckedContiguousConstIterator<Value> pos,
    Value&& value) {
  CR_CHECK(is_list());
  const auto offset = pos - MakeSpan(list_).begin();
  list_.insert(list_.begin() + offset, std::move(value));
  return MakeSpan(list_).begin() + offset;
}

bool Value::EraseListIter(
    internal::CheckedContiguousConstIterator<Value> iter) {
  CR_CHECK(is_list());
  const auto offset = iter - MakeSpan(list_).begin();
  auto list_iter = list_.begin() + offset;
  if (list_iter == list_.end())
    return false;

  list_.erase(list_iter);
  return true;
}

size_t Value::EraseListValue(const Value& val) {
  return EraseListValueIf([&val](const Value& other) { return val == other; });
}

void Value::ClearList() {
  CR_CHECK(is_list());
  list_.clear();
}

Value* Value::FindKey(StringPiece key) {
  return const_cast<Value*>(as_const(*this).FindKey(key));
}

const Value* Value::FindKey(StringPiece key) const {
  CR_CHECK(is_dict());
  auto found = dict_.find(key);
  if (found == dict_.end())
    return nullptr;
  return found->second.get();
}

Value* Value::FindKeyOfType(StringPiece key, Type type) {
  return const_cast<Value*>(as_const(*this).FindKeyOfType(key, type));
}

const Value* Value::FindKeyOfType(StringPiece key, Type type) const {
  const Value* result = FindKey(key);
  if (!result || result->type() != type)
    return nullptr;
  return result;
}

cr::Optional<bool> Value::FindBoolKey(StringPiece key) const {
  const Value* result = FindKeyOfType(key, Type::BOOLEAN);
  return result ? cr::MakeOptional(result->bool_value_) : cr::nullopt;
}

cr::Optional<int> Value::FindIntKey(StringPiece key) const {
  const Value* result = FindKeyOfType(key, Type::INTEGER);
  return result ? cr::MakeOptional(result->int_value_) : cr::nullopt;
}

cr::Optional<double> Value::FindDoubleKey(StringPiece key) const {
  const Value* result = FindKey(key);
  if (result) {
    if (result->is_int())
      return static_cast<double>(result->int_value_);
    if (result->is_double()) {
      return result->AsDoubleInternal();
    }
  }
  return cr::nullopt;
}

const std::string* Value::FindStringKey(StringPiece key) const {
  const Value* result = FindKeyOfType(key, Type::STRING);
  return result ? &result->string_value_ : nullptr;
}

std::string* Value::FindStringKey(StringPiece key) {
  Value* result = FindKeyOfType(key, Type::STRING);
  return result ? &result->string_value_ : nullptr;
}

const Value::BlobStorage* Value::FindBlobKey(StringPiece key) const {
  const Value* value = FindKeyOfType(key, Type::BINARY);
  return value ? &value->binary_value_ : nullptr;
}

const Value* Value::FindDictKey(StringPiece key) const {
  return FindKeyOfType(key, Type::DICTIONARY);
}

Value* Value::FindDictKey(StringPiece key) {
  return FindKeyOfType(key, Type::DICTIONARY);
}

const Value* Value::FindListKey(StringPiece key) const {
  return FindKeyOfType(key, Type::LIST);
}

Value* Value::FindListKey(StringPiece key) {
  return FindKeyOfType(key, Type::LIST);
}

Value* Value::SetKey(StringPiece key, Value&& value) {
  return SetKeyInternal(key, std::make_unique<Value>(std::move(value)));
}

Value* Value::SetKey(std::string&& key, Value&& value) {
  CR_CHECK(is_dict());
  return dict_
      .insert_or_assign(std::move(key),
                        std::make_unique<Value>(std::move(value)))
      .first->second.get();
}

Value* Value::SetKey(const char* key, Value&& value) {
  return SetKeyInternal(key, std::make_unique<Value>(std::move(value)));
}

Value* Value::SetBoolKey(StringPiece key, bool value) {
  return SetKeyInternal(key, std::make_unique<Value>(value));
}

Value* Value::SetIntKey(StringPiece key, int value) {
  return SetKeyInternal(key, std::make_unique<Value>(value));
}

Value* Value::SetDoubleKey(StringPiece key, double value) {
  return SetKeyInternal(key, std::make_unique<Value>(value));
}

Value* Value::SetStringKey(StringPiece key, StringPiece value) {
  return SetKeyInternal(key, std::make_unique<Value>(value));
}

Value* Value::SetStringKey(StringPiece key, const char* value) {
  return SetKeyInternal(key, std::make_unique<Value>(value));
}

Value* Value::SetStringKey(StringPiece key, std::string&& value) {
  return SetKeyInternal(key, std::make_unique<Value>(std::move(value)));
}

Value* Value::SetStringKey(StringPiece key, StringPiece16 value) {
  return SetKeyInternal(key, std::make_unique<Value>(value));
}

bool Value::RemoveKey(StringPiece key) {
  CR_CHECK(is_dict());
  return dict_.erase(key) != 0;
}

Optional<Value> Value::ExtractKey(StringPiece key) {
  CR_CHECK(is_dict());
  auto found = dict_.find(key);
  if (found == dict_.end())
    return nullopt;

  Value value = std::move(*found->second);
  dict_.erase(found);
  return std::move(value);
}

Value* Value::FindPath(StringPiece path) {
  return const_cast<Value*>(as_const(*this).FindPath(path));
}

const Value* Value::FindPath(StringPiece path) const {
  CR_CHECK(is_dict());
  const Value* cur = this;
  PathSplitter splitter(path);
  while (splitter.HasNext()) {
    if (!cur->is_dict() || (cur = cur->FindKey(splitter.Next())) == nullptr)
      return nullptr;
  }
  return cur;
}

Value* Value::FindPathOfType(StringPiece path, Type type) {
  return const_cast<Value*>(as_const(*this).FindPathOfType(path, type));
}

const Value* Value::FindPathOfType(StringPiece path, Type type) const {
  const Value* cur = FindPath(path);
  if (!cur || cur->type() != type)
    return nullptr;
  return cur;
}

cr::Optional<bool> Value::FindBoolPath(StringPiece path) const {
  const Value* cur = FindPath(path);
  if (!cur || !cur->is_bool())
    return cr::nullopt;
  return cur->bool_value_;
}

cr::Optional<int> Value::FindIntPath(StringPiece path) const {
  const Value* cur = FindPath(path);
  if (!cur || !cur->is_int())
    return cr::nullopt;
  return cur->int_value_;
}

cr::Optional<double> Value::FindDoublePath(StringPiece path) const {
  const Value* cur = FindPath(path);
  if (cur) {
    if (cur->is_int())
      return static_cast<double>(cur->int_value_);
    if (cur->is_double())
      return cur->AsDoubleInternal();
  }
  return cr::nullopt;
}

const std::string* Value::FindStringPath(StringPiece path) const {
  const Value* cur = FindPath(path);
  if (!cur || !cur->is_string())
    return nullptr;
  return &cur->string_value_;
}

std::string* Value::FindStringPath(StringPiece path) {
  return const_cast<std::string*>(as_const(*this).FindStringPath(path));
}

const Value::BlobStorage* Value::FindBlobPath(StringPiece path) const {
  const Value* cur = FindPath(path);
  if (!cur || !cur->is_blob())
    return nullptr;
  return &cur->binary_value_;
}

const Value* Value::FindDictPath(StringPiece path) const {
  return FindPathOfType(path, Type::DICTIONARY);
}

Value* Value::FindDictPath(StringPiece path) {
  return FindPathOfType(path, Type::DICTIONARY);
}

const Value* Value::FindListPath(StringPiece path) const {
  return FindPathOfType(path, Type::LIST);
}

Value* Value::FindListPath(StringPiece path) {
  return FindPathOfType(path, Type::LIST);
}

Value* Value::SetPath(StringPiece path, Value&& value) {
  return SetPathInternal(path, std::make_unique<Value>(std::move(value)));
}

Value* Value::SetBoolPath(StringPiece path, bool value) {
  return SetPathInternal(path, std::make_unique<Value>(value));
}

Value* Value::SetIntPath(StringPiece path, int value) {
  return SetPathInternal(path, std::make_unique<Value>(value));
}

Value* Value::SetDoublePath(StringPiece path, double value) {
  return SetPathInternal(path, std::make_unique<Value>(value));
}

Value* Value::SetStringPath(StringPiece path, StringPiece value) {
  return SetPathInternal(path, std::make_unique<Value>(value));
}

Value* Value::SetStringPath(StringPiece path, std::string&& value) {
  return SetPathInternal(path, std::make_unique<Value>(std::move(value)));
}

Value* Value::SetStringPath(StringPiece path, const char* value) {
  return SetPathInternal(path, std::make_unique<Value>(value));
}

Value* Value::SetStringPath(StringPiece path, StringPiece16 value) {
  return SetPathInternal(path, std::make_unique<Value>(value));
}

bool Value::RemovePath(StringPiece path) {
  return ExtractPath(path).has_value();
}

Optional<Value> Value::ExtractPath(StringPiece path) {
  if (!is_dict() || path.empty())
    return nullopt;

  // NOTE: PathSplitter is not being used here because recursion is used to
  // ensure that dictionaries that become empty due to this operation are
  // removed automatically.
  size_t pos = path.find('.');
  if (pos == path.npos)
    return ExtractKey(path);

  auto found = dict_.find(path.substr(0, pos));
  if (found == dict_.end() || !found->second->is_dict())
    return nullopt;

  Optional<Value> extracted = found->second->ExtractPath(path.substr(pos + 1));
  if (extracted && found->second->dict_.empty())
    dict_.erase(found);

  return extracted;
}

// DEPRECATED METHODS
Value* Value::FindPath(std::initializer_list<StringPiece> path) {
  return const_cast<Value*>(as_const(*this).FindPath(path));
}

Value* Value::FindPath(Span<const StringPiece> path) {
  return const_cast<Value*>(as_const(*this).FindPath(path));
}

const Value* Value::FindPath(std::initializer_list<StringPiece> path) const {
  CR_DCHECK(path.size() >= 2u) << "Use FindKey() for a path of length 1.";
  return FindPath(MakeSpan(path.begin(), path.size()));
}

const Value* Value::FindPath(Span<const StringPiece> path) const {
  const Value* cur = this;
  for (const StringPiece component : path) {
    if (!cur->is_dict() || (cur = cur->FindKey(component)) == nullptr)
      return nullptr;
  }
  return cur;
}

Value* Value::FindPathOfType(std::initializer_list<StringPiece> path,
                             Type type) {
  return const_cast<Value*>(as_const(*this).FindPathOfType(path, type));
}

Value* Value::FindPathOfType(Span<const StringPiece> path, Type type) {
  return const_cast<Value*>(as_const(*this).FindPathOfType(path, type));
}

const Value* Value::FindPathOfType(std::initializer_list<StringPiece> path,
                                   Type type) const {
  CR_DCHECK(path.size() >= 2u) << "Use FindKeyOfType() for a path of length 1.";
  return FindPathOfType(MakeSpan(path.begin(), path.size()), type);
}

const Value* Value::FindPathOfType(Span<const StringPiece> path,
                                   Type type) const {
  const Value* result = FindPath(path);
  if (!result || result->type() != type)
    return nullptr;
  return result;
}

Value::dict_iterator_proxy Value::DictItems() {
  CR_CHECK(is_dict());
  return dict_iterator_proxy(&dict_);
}

Value::const_dict_iterator_proxy Value::DictItems() const {
  CR_CHECK(is_dict());
  return const_dict_iterator_proxy(&dict_);
}

size_t Value::DictSize() const {
  CR_CHECK(is_dict());
  return dict_.size();
}

bool Value::DictEmpty() const {
  CR_CHECK(is_dict());
  return dict_.empty();
}

void Value::MergeDictionary(const Value* dictionary) {
  CR_CHECK(is_dict());
  CR_CHECK(dictionary->is_dict());
  for (const auto& pair : dictionary->dict_) {
    const auto& key = pair.first;
    const auto& val = pair.second;
    // Check whether we have to merge dictionaries.
    if (val->is_dict()) {
      auto found = dict_.find(key);
      if (found != dict_.end() && found->second->is_dict()) {
        found->second->MergeDictionary(val.get());
        continue;
      }
    }

    // All other cases: Make a copy and hook it up.
    SetKey(key, val->Clone());
  }
}

bool Value::GetAsDictionary(DictionaryValue** out_value) {
  if (out_value && is_dict()) {
    *out_value = static_cast<DictionaryValue*>(this);
    return true;
  }
  return is_dict();
}

bool Value::GetAsDictionary(const DictionaryValue** out_value) const {
  if (out_value && is_dict()) {
    *out_value = static_cast<const DictionaryValue*>(this);
    return true;
  }
  return is_dict();
}


bool operator==(const Value& lhs, const Value& rhs) {
  if (lhs.type_ != rhs.type_)
    return false;

  switch (lhs.type_) {
    case Value::Type::NONE:
      return true;
    case Value::Type::BOOLEAN:
      return lhs.bool_value_ == rhs.bool_value_;
    case Value::Type::INTEGER:
      return lhs.int_value_ == rhs.int_value_;
    case Value::Type::DOUBLE:
      return lhs.AsDoubleInternal() == rhs.AsDoubleInternal();
    case Value::Type::STRING:
      return lhs.string_value_ == rhs.string_value_;
    case Value::Type::BINARY:
      return lhs.binary_value_ == rhs.binary_value_;
    // TODO(crbug.com/646113): Clean this up when DictionaryValue and ListValue
    // are completely inlined.
    case Value::Type::DICTIONARY:
      if (lhs.dict_.size() != rhs.dict_.size())
        return false;
      return std::equal(std::begin(lhs.dict_), std::end(lhs.dict_),
                        std::begin(rhs.dict_),
                        [](const auto& u, const auto& v) {
                          return std::tie(u.first, *u.second) ==
                                 std::tie(v.first, *v.second);
                        });
    case Value::Type::LIST:
      return lhs.list_ == rhs.list_;
      // TODO(crbug.com/859477): Remove after root cause is found.
    case Value::Type::DEAD:
      CR_CHECK(false);
      return false;
  }

  // TODO(crbug.com/859477): Revert to NOTREACHED() after root cause is found.
  CR_CHECK(false);
  return false;
}

bool operator!=(const Value& lhs, const Value& rhs) {
  return !(lhs == rhs);
}

bool operator<(const Value& lhs, const Value& rhs) {
  if (lhs.type_ != rhs.type_)
    return lhs.type_ < rhs.type_;

  switch (lhs.type_) {
    case Value::Type::NONE:
      return false;
    case Value::Type::BOOLEAN:
      return lhs.bool_value_ < rhs.bool_value_;
    case Value::Type::INTEGER:
      return lhs.int_value_ < rhs.int_value_;
    case Value::Type::DOUBLE:
      return lhs.AsDoubleInternal() < rhs.AsDoubleInternal();
    case Value::Type::STRING:
      return lhs.string_value_ < rhs.string_value_;
    case Value::Type::BINARY:
      return lhs.binary_value_ < rhs.binary_value_;
    // TODO(crbug.com/646113): Clean this up when DictionaryValue and ListValue
    // are completely inlined.
    case Value::Type::DICTIONARY:
      return std::lexicographical_compare(
          std::begin(lhs.dict_), std::end(lhs.dict_), std::begin(rhs.dict_),
          std::end(rhs.dict_),
          [](const Value::DictStorage::value_type& u,
             const Value::DictStorage::value_type& v) {
            return std::tie(u.first, *u.second) < std::tie(v.first, *v.second);
          });
    case Value::Type::LIST:
      return lhs.list_ < rhs.list_;
      // TODO(crbug.com/859477): Remove after root cause is found.
    case Value::Type::DEAD:
      CR_CHECK(false);
      return false;
  }

  // TODO(crbug.com/859477): Revert to NOTREACHED() after root cause is found.
  CR_CHECK(false);
  return false;
}

bool operator>(const Value& lhs, const Value& rhs) {
  return rhs < lhs;
}

bool operator<=(const Value& lhs, const Value& rhs) {
  return !(rhs < lhs);
}

bool operator>=(const Value& lhs, const Value& rhs) {
  return !(lhs < rhs);
}

bool Value::Equals(const Value* other) const {
  CR_DCHECK(other);
  return *this == *other;
}

size_t Value::EstimateMemoryUsage() const {
  switch (type_) {
    case Type::STRING:
      return string_value_.capacity() + sizeof(string_value_);
    case Type::BINARY:
      return binary_value_.capacity() + sizeof(binary_value_);
    case Type::DICTIONARY:
      return dict_.capacity() + sizeof(dict_);
    case Type::LIST:
      return list_.capacity() + sizeof(list_);
    default:
      return 0;
  }
}

void Value::InternalMoveConstructFrom(Value&& that) {
  type_ = that.type_;

  switch (type_) {
    case Type::NONE:
      return;
    case Type::BOOLEAN:
      bool_value_ = that.bool_value_;
      return;
    case Type::INTEGER:
      int_value_ = that.int_value_;
      return;
    case Type::DOUBLE:
      double_value_ = that.double_value_;
      return;
    case Type::STRING:
      new (&string_value_) std::string(std::move(that.string_value_));
      return;
    case Type::BINARY:
      new (&binary_value_) BlobStorage(std::move(that.binary_value_));
      return;
    case Type::DICTIONARY:
      new (&dict_) DictStorage(std::move(that.dict_));
      return;
    case Type::LIST:
      new (&list_) ListStorage(std::move(that.list_));
      return;
      // TODO(crbug.com/859477): Remove after root cause is found.
    case Type::DEAD:
      CR_CHECK(false);
      return;
  }

  // TODO(crbug.com/859477): Revert to NOTREACHED() after root cause is found.
  CR_CHECK(false);
}

void Value::InternalCleanup() {
  switch (type_) {
    case Type::NONE:
    case Type::BOOLEAN:
    case Type::INTEGER:
    case Type::DOUBLE:
      // Nothing to do
      return;

    case Type::STRING:
      string_value_.~basic_string();
      return;
    case Type::BINARY:
      binary_value_.~BlobStorage();
      return;
    case Type::DICTIONARY:
      dict_.~DictStorage();
      return;
    case Type::LIST:
      list_.~ListStorage();
      return;
      // TODO(crbug.com/859477): Remove after root cause is found.
    case Type::DEAD:
      CR_CHECK(false);
      return;
  }

  // TODO(crbug.com/859477): Revert to NOTREACHED() after root cause is found.
  CR_CHECK(false);
}

Value* Value::SetKeyInternal(StringPiece key,
                             std::unique_ptr<Value>&& val_ptr) {
  CR_CHECK(is_dict());
  // NOTE: We can't use |insert_or_assign| here, as only |try_emplace| does
  // an explicit conversion from StringPiece to std::string if necessary.
  auto result = dict_.try_emplace(key, std::move(val_ptr));
  if (!result.second) {
    // val_ptr is guaranteed to be still intact at this point.
    result.first->second = std::move(val_ptr);
  }
  return result.first->second.get();
}

Value* Value::SetPathInternal(StringPiece path,
                              std::unique_ptr<Value>&& value_ptr) {
  PathSplitter splitter(path);
  CR_DCHECK(splitter.HasNext()) << "Cannot call SetPath() with empty path";
  // Walk/construct intermediate dictionaries. The last element requires
  // special handling so skip it in this loop.
  Value* cur = this;
  StringPiece path_component = splitter.Next();
  while (splitter.HasNext()) {
    if (!cur->is_dict())
      return nullptr;

    // Use lower_bound to avoid doing the search twice for missing keys.
    auto found = cur->dict_.lower_bound(path_component);
    if (found == cur->dict_.end() || found->first != path_component) {
      // No key found, insert one.
      auto inserted = cur->dict_.try_emplace(
          found, path_component, std::make_unique<Value>(Type::DICTIONARY));
      cur = inserted->second.get();
    } else {
      cur = found->second.get();
    }
    path_component = splitter.Next();
  }

  // "cur" will now contain the last dictionary to insert or replace into.
  if (!cur->is_dict())
    return nullptr;
  return cur->SetKeyInternal(path_component, std::move(value_ptr));
}

///////////////////// DictionaryValue ////////////////////

// static
std::unique_ptr<DictionaryValue> DictionaryValue::From(
    std::unique_ptr<Value> value) {
  DictionaryValue* out;
  if (value && value->GetAsDictionary(&out)) {
    CR_IGNORE_RESULT(value.release());
    return WrapUnique(out);
  }
  return nullptr;
}

DictionaryValue::DictionaryValue() : Value(Type::DICTIONARY) {}
DictionaryValue::DictionaryValue(const DictStorage& in_dict) : Value(in_dict) {}
DictionaryValue::DictionaryValue(DictStorage&& in_dict) noexcept
    : Value(std::move(in_dict)) {}

void DictionaryValue::Clear() {
  dict_.clear();
}

std::unique_ptr<DictionaryValue> DictionaryValue::DeepCopyWithoutEmptyChildren()
    const {
  std::unique_ptr<DictionaryValue> copy =
      CopyDictionaryWithoutEmptyChildren(*this);
  if (!copy)
    copy = std::make_unique<DictionaryValue>();
  return copy;
}

void DictionaryValue::Swap(DictionaryValue* other) {
  CR_CHECK(other->is_dict());
  dict_.swap(other->dict_);
}

///////////////////// ListValue ////////////////////

// static
std::unique_ptr<ListValue> ListValue::From(std::unique_ptr<Value> value) {
  ListValue* out = nullptr;
  if (value && value->is_list()) {
    out = static_cast<ListValue*>(value.get());
  }

  if (out) {
    CR_IGNORE_RESULT(value.release());
    return WrapUnique(out);
  }
  return nullptr;
}

ListValue::ListValue() : Value(Type::LIST) {}
ListValue::ListValue(Span<const Value> in_list) : Value(in_list) {}
ListValue::ListValue(ListStorage&& in_list) noexcept
    : Value(std::move(in_list)) {}

bool ListValue::GetDictionary(size_t index,
                              const DictionaryValue** out_value) const {
  if (index >= GetList().size())
    return false;

  const Value* value = &GetList()[index];
  if (!value->is_dict())
    return false;

  if (out_value)
    *out_value = static_cast<const DictionaryValue*>(value);

  return true;
}

bool ListValue::GetDictionary(size_t index, DictionaryValue** out_value) {
  return as_const(*this).GetDictionary(
      index, const_cast<const DictionaryValue**>(out_value));
}

ValueSerializer::~ValueSerializer() = default;

ValueDeserializer::~ValueDeserializer() = default;

std::ostream& operator<<(std::ostream& out, const Value& value) {
  std::string json;
  JSONWriter::WriteWithOptions(value, JSONWriter::OPTIONS_PRETTY_PRINT, &json);
  return out << json;
}

std::ostream& operator<<(std::ostream& out, const Value::Type& type) {
  if (static_cast<int>(type) < 0 ||
      static_cast<size_t>(type) >= cr::size(kTypeNames))
    return out << "Invalid Type (index = " << static_cast<int>(type) << ")";
  return out << Value::GetTypeName(type);
}

}  // namespace cr
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file specifies a recursive data storage class called Value intended for
// storing settings and other persistable data.
//
// A Value represents something that can be stored in JSON or passed to/from
// JavaScript. As such, it is NOT a generalized variant type, since only the
// types supported by JavaScript/JSON are supported.
//
// IN PARTICULAR this means that there is no support for int64_t or unsigned
// numbers. Writing JSON with such types would violate the spec. If you need
// something like this, either use a double or make a string value containing
// the number you want.
//
// NOTE: A Value parameter that is always a Value::STRING should just be passed
// as a std::string. Similarly for Values that are always Value::DICTIONARY
// (should be flat_map), Value::LIST (should be std::vector), et cetera.

#ifndef MINI_CHROMIUM_SRC_CRBASE_VALUES_H_
#define MINI_CHROMIUM_SRC_CRBASE_VALUES_H_

#include <stddef.h>
#include <stdint.h>

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cr_base/base_export.h"
#include "cr_base/containers/internal/checked_iterators.h"
#include "cr_base/containers/internal/checked_range.h"
#include "cr_base/containers/flat_map.h"
#include "cr_base/containers/span.h"
#include "cr_base/containers/optional.h"
#include "cr_base/strings/string_piece.h"
#include "cr_base/value_iterators.h"

namespace cr {

class DictionaryValue;
class ListValue;
class Value;

// The Value class is the base class for Values. A Value can be instantiated
// via passing the appropriate type or backing storage to the constructor.
//
// See the file-level comment above for more information.
//
// base::Value is currently in the process of being refactored. Design doc:
// https://docs.google.com/document/d/1uDLu5uTRlCWePxQUEHc8yNQdEoE1BDISYdpggWEABnw
//
// Previously (which is how most code that currently exists is written), Value
// used derived types to implement the individual data types, and base::Value
// was just a base class to refer to them. This required everything be heap
// allocated.
//
// OLD WAY:
//
//   std::unique_ptr<base::Value> GetFoo() {
//     std::unique_ptr<DictionaryValue> dict;
//     dict->SetString("mykey", foo);
//     return dict;
//   }
//
// The new design makes base::Value a variant type that holds everything in
// a union. It is now recommended to pass by value with std::move rather than
// use heap allocated values. The DictionaryValue and ListValue subclasses
// exist only as a compatibility shim that we're in the process of removing.
//
// NEW WAY:
//
//   cr::Value GetFoo() {
//     cr::Value dict(cr::Value::Type::DICTIONARY);
//     dict.SetKey("mykey", cr::Value(foo));
//     return dict;
//   }
class CRBASE_EXPORT Value {
 public:
  using BlobStorage = std::vector<uint8_t>;
  using DictStorage = FlatMap<std::string, std::unique_ptr<Value>>;
  using ListStorage = std::vector<Value>;

  using ListView = internal::CheckedContiguousRange<ListStorage>;
  using ConstListView = internal::CheckedContiguousConstRange<ListStorage>;

  // See technical note below explaining why this is used.
  using DoubleStorage = struct { alignas(4) char v[sizeof(double)]; };

  enum class Type : unsigned char {
    NONE = 0,
    BOOLEAN,
    INTEGER,
    DOUBLE,
    STRING,
    BINARY,
    DICTIONARY,
    LIST,
    // TODO(crbug.com/859477): Remove once root cause is found.
    DEAD
    // Note: Do not add more types. See the file-level comment above for why.
  };

  // Adaptors for converting from the old way to the new way and vice versa.
  static Value FromUniquePtrValue(std::unique_ptr<Value> val);
  static std::unique_ptr<Value> ToUniquePtrValue(Value val);
  static const DictionaryValue& AsDictionaryValue(const Value& val);
  static const ListValue& AsListValue(const Value& val);

  Value(const Value&) = delete;
  Value& operator=(const Value&) = delete;

  Value(Value&& that) noexcept;
  Value() noexcept {}  // A null value
  // Fun fact: using '= default' above instead of '{}' does not work because
  // the compiler complains that the default constructor was deleted since
  // the inner union contains fields with non-default constructors.

  // Value's copy constructor and copy assignment operator are deleted. Use this
  // to obtain a deep copy explicitly.
  Value Clone() const;

  explicit Value(Type type);
  explicit Value(bool in_bool);
  explicit Value(int in_int);
  explicit Value(double in_double);

  // Value(const char*) and Value(const char16*) are required despite
  // Value(StringPiece) and Value(StringPiece16) because otherwise the
  // compiler will choose the Value(bool) constructor for these arguments.
  // Value(std::string&&) allow for efficient move construction.
  explicit Value(const char* in_string);
  explicit Value(StringPiece in_string);
  explicit Value(std::string&& in_string) noexcept;
  explicit Value(const char16_t* in_string16);
  explicit Value(StringPiece16 in_string16);

  explicit Value(const std::vector<char>& in_blob);
  explicit Value(Span<const uint8_t> in_blob);
  explicit Value(BlobStorage&& in_blob) noexcept;

  explicit Value(const DictStorage& in_dict);
  explicit Value(DictStorage&& in_dict) noexcept;

  explicit Value(Span<const Value> in_list);
  explicit Value(ListStorage&& in_list) noexcept;

  Value& operator=(Value&& that) noexcept;

  ~Value();

  // Returns the name for a given |type|.
  static const char* GetTypeName(Type type);

  // Returns the type of the value stored by the current Value object.
  Type type() const { return type_; }

  // Returns true if the current object represents a given type.
  bool is_none() const { return type() == Type::NONE; }
  bool is_bool() const { return type() == Type::BOOLEAN; }
  bool is_int() const { return type() == Type::INTEGER; }
  bool is_double() const { return type() == Type::DOUBLE; }
  bool is_string() const { return type() == Type::STRING; }
  bool is_blob() const { return type() == Type::BINARY; }
  bool is_dict() const { return type() == Type::DICTIONARY; }
  bool is_list() const { return type() == Type::LIST; }

  // These will all CHECK that the type matches.
  bool GetBool() const;
  int GetInt() const;
  double GetDouble() const;  // Implicitly converts from int if necessary.
  const std::string& GetString() const;
  std::string& GetString();
  const BlobStorage& GetBlob() const;

  // Returns the Values in a list as a view. The mutable overload allows for
  // modification of the underlying values, but does not allow changing the
  // structure of the list. If this is desired, use TakeList(), perform the
  // operations, and return the list back to the Value via move assignment.
  ListView GetList();
  ConstListView GetList() const;

  // Transfers ownership of the the underlying list to the caller. Subsequent
  // calls to GetList() will return an empty list.
  // Note: This CHECKs that type() is Type::LIST.
  ListStorage TakeList();

  // Appends |value| to the end of the list.
  // Note: These CHECK that type() is Type::LIST.
  void Append(bool value);
  void Append(int value);
  void Append(double value);
  void Append(const char* value);
  void Append(StringPiece value);
  void Append(std::string&& value);
  void Append(const char16_t* value);
  void Append(StringPiece16 value);
  void Append(Value&& value);

  // Inserts |value| before |pos|.
  // Note: This CHECK that type() is Type::LIST.
  internal::CheckedContiguousIterator<Value> Insert(
    internal::CheckedContiguousConstIterator<Value> pos,
      Value&& value);

  // Erases the Value pointed to by |iter|. Returns false if |iter| is out of
  // bounds.
  // Note: This CHECKs that type() is Type::LIST.
  bool EraseListIter(internal::CheckedContiguousConstIterator<Value> iter);

  // Erases all Values that compare equal to |val|. Returns the number of
  // deleted Values.
  // Note: This CHECKs that type() is Type::LIST.
  size_t EraseListValue(const Value& val);

  // Erases all Values for which |pred| returns true. Returns the number of
  // deleted Values.
  // Note: This CHECKs that type() is Type::LIST.
  template <typename Predicate>
  size_t EraseListValueIf(Predicate pred) {
    CR_CHECK(is_list());
    const size_t old_size = list_.size();
    cr::erase_if(list_, pred);
    return old_size - list_.size();
  }

  // Erases all Values from the list.
  // Note: This CHECKs that type() is Type::LIST.
  void ClearList();

  // |FindKey| looks up |key| in the underlying dictionary. If found, it returns
  // a pointer to the element. Otherwise it returns nullptr.
  // returned. Callers are expected to perform a check against null before using
  // the pointer.
  // Note: This CHECKs that type() is Type::DICTIONARY.
  //
  // Example:
  //   auto* found = FindKey("foo");
  Value* FindKey(StringPiece key);
  const Value* FindKey(StringPiece key) const;

  // |FindKeyOfType| is similar to |FindKey|, but it also requires the found
  // value to have type |type|. If no type is found, or the found value is of a
  // different type nullptr is returned.
  // Callers are expected to perform a check against null before using the
  // pointer.
  // Note: This CHECKs that type() is Type::DICTIONARY.
  //
  // Example:
  //   auto* found = FindKey("foo", Type::DOUBLE);
  Value* FindKeyOfType(StringPiece key, Type type);
  const Value* FindKeyOfType(StringPiece key, Type type) const;

  // These are convenience forms of |FindKey|. They return |base::nullopt| if
  // the value is not found or doesn't have the type specified in the
  // function's name.
  cr::Optional<bool> FindBoolKey(StringPiece key) const;
  cr::Optional<int> FindIntKey(StringPiece key) const;
  // Note FindDoubleKey() will auto-convert INTEGER keys to their double
  // value, for consistency with GetDouble().
  cr::Optional<double> FindDoubleKey(StringPiece key) const;

  // |FindStringKey| returns |nullptr| if value is not found or not a string.
  const std::string* FindStringKey(StringPiece key) const;
  std::string* FindStringKey(StringPiece key);

  // Returns nullptr is value is not found or not a binary.
  const BlobStorage* FindBlobKey(StringPiece key) const;

  // Returns nullptr if value is not found or not a dictionary.
  const Value* FindDictKey(StringPiece key) const;
  Value* FindDictKey(StringPiece key);

  // Returns nullptr if value is not found or not a list.
  const Value* FindListKey(StringPiece key) const;
  Value* FindListKey(StringPiece key);

  // |SetKey| looks up |key| in the underlying dictionary and sets the mapped
  // value to |value|. If |key| could not be found, a new element is inserted.
  // A pointer to the modified item is returned.
  // Note: This CHECKs that type() is Type::DICTIONARY.
  // Note: Prefer Set<Type>Key() for simple values.
  //
  // Example:
  //   SetKey("foo", std::move(myvalue));
  Value* SetKey(StringPiece key, Value&& value);
  // This overload results in a performance improvement for std::string&&.
  Value* SetKey(std::string&& key, Value&& value);
  // This overload is necessary to avoid ambiguity for const char* arguments.
  Value* SetKey(const char* key, Value&& value);

  // |Set<Type>Key| looks up |key| in the underlying dictionary and associates
  // a corresponding Value() constructed from the second parameter. Compared
  // to SetKey(), this avoids un-necessary temporary Value() creation, as well
  // ambiguities in the value type.
  Value* SetBoolKey(StringPiece key, bool val);
  Value* SetIntKey(StringPiece key, int val);
  Value* SetDoubleKey(StringPiece key, double val);
  Value* SetStringKey(StringPiece key, StringPiece val);
  // NOTE: These two overloads are provided as performance / code generation
  // optimizations.
  Value* SetStringKey(StringPiece key, const char* val);
  Value* SetStringKey(StringPiece key, std::string&& val);
  Value* SetStringKey(StringPiece key, StringPiece16 val);

  // This attempts to remove the value associated with |key|. In case of
  // failure, e.g. the key does not exist, false is returned and the underlying
  // dictionary is not changed. In case of success, |key| is deleted from the
  // dictionary and the method returns true.
  // Note: This CHECKs that type() is Type::DICTIONARY.
  //
  // Example:
  //   bool success = dict.RemoveKey("foo");
  bool RemoveKey(StringPiece key);

  // This attempts to extract the value associated with |key|. In case of
  // failure, e.g. the key does not exist, nullopt is returned and the
  // underlying dictionary is not changed. In case of success, |key| is deleted
  // from the dictionary and the method returns the extracted Value.
  // Note: This CHECKs that type() is Type::DICTIONARY.
  //
  // Example:
  //   Optional<Value> maybe_value = dict.ExtractKey("foo");
  Optional<Value> ExtractKey(StringPiece key);

  // Searches a hierarchy of dictionary values for a given value. If a path
  // of dictionaries exist, returns the item at that path. If any of the path
  // components do not exist or if any but the last path components are not
  // dictionaries, returns nullptr.
  //
  // The type of the leaf Value is not checked.
  //
  // Implementation note: This can't return an iterator because the iterator
  // will actually be into another Value, so it can't be compared to iterators
  // from this one (in particular, the DictItems().end() iterator).
  //
  // This version takes a StringPiece for the path, using dots as separators.
  // Example:
  //    auto* found = FindPath("foo.bar");
  Value* FindPath(StringPiece path);
  const Value* FindPath(StringPiece path) const;

  // There are also deprecated versions that take the path parameter
  // as either a std::initializer_list<StringPiece> or a
  // span<const StringPiece>. The latter is useful to use a
  // std::vector<std::string> as a parameter but creates huge dynamic
  // allocations and should be avoided!
  // Note: If there is only one component in the path, use FindKey() instead.
  //
  // Example:
  //   std::vector<StringPiece> components = ...
  //   auto* found = FindPath(components);
  Value* FindPath(std::initializer_list<StringPiece> path);
  Value* FindPath(Span<const StringPiece> path);
  const Value* FindPath(std::initializer_list<StringPiece> path) const;
  const Value* FindPath(Span<const StringPiece> path) const;

  // Like FindPath() but will only return the value if the leaf Value type
  // matches the given type. Will return nullptr otherwise.
  // Note: Prefer Find<Type>Path() for simple values.
  //
  // Note: If there is only one component in the path, use FindKeyOfType()
  // instead for slightly better performance.
  Value* FindPathOfType(StringPiece path, Type type);
  const Value* FindPathOfType(StringPiece path, Type type) const;

  // Convenience accessors used when the expected type of a value is known.
  // Similar to Find<Type>Key() but accepts paths instead of keys.
  cr::Optional<bool> FindBoolPath(StringPiece path) const;
  cr::Optional<int> FindIntPath(StringPiece path) const;
  cr::Optional<double> FindDoublePath(StringPiece path) const;
  const std::string* FindStringPath(StringPiece path) const;
  std::string* FindStringPath(StringPiece path);
  const BlobStorage* FindBlobPath(StringPiece path) const;
  Value* FindDictPath(StringPiece path);
  const Value* FindDictPath(StringPiece path) const;
  Value* FindListPath(StringPiece path);
  const Value* FindListPath(StringPiece path) const;

  // The following forms are deprecated too, use the ones that take the path
  // as a single StringPiece instead.
  Value* FindPathOfType(std::initializer_list<StringPiece> path, Type type);
  Value* FindPathOfType(Span<const StringPiece> path, Type type);
  const Value* FindPathOfType(std::initializer_list<StringPiece> path,
                              Type type) const;
  const Value* FindPathOfType(Span<const StringPiece> path, Type type) const;

  // Sets the given path, expanding and creating dictionary keys as necessary.
  //
  // If the current value is not a dictionary, the function returns nullptr. If
  // path components do not exist, they will be created. If any but the last
  // components matches a value that is not a dictionary, the function will fail
  // (it will not overwrite the value) and return nullptr. The last path
  // component will be unconditionally overwritten if it exists, and created if
  // it doesn't.
  //
  // Example:
  //   value.SetPath("foo.bar", std::move(myvalue));
  //
  // Note: If there is only one component in the path, use SetKey() instead.
  // Note: Using Set<Type>Path() might be more convenient and efficient.
  Value* SetPath(StringPiece path, Value&& value);

  // These setters are more convenient and efficient than the corresponding
  // SetPath(...) call.
  Value* SetBoolPath(StringPiece path, bool value);
  Value* SetIntPath(StringPiece path, int value);
  Value* SetDoublePath(StringPiece path, double value);
  Value* SetStringPath(StringPiece path, StringPiece value);
  Value* SetStringPath(StringPiece path, const char* value);
  Value* SetStringPath(StringPiece path, std::string&& value);
  Value* SetStringPath(StringPiece path, StringPiece16 value);

  // Tries to remove a Value at the given path.
  //
  // If the current value is not a dictionary or any path component does not
  // exist, this operation fails, leaves underlying Values untouched and returns
  // |false|. In case intermediate dictionaries become empty as a result of this
  // path removal, they will be removed as well.
  // Note: If there is only one component in the path, use ExtractKey() instead.
  //
  // Example:
  //   bool success = value.RemovePath("foo.bar");
  bool RemovePath(StringPiece path);

  // Tries to extract a Value at the given path.
  //
  // If the current value is not a dictionary or any path component does not
  // exist, this operation fails, leaves underlying Values untouched and returns
  // nullopt. In case intermediate dictionaries become empty as a result of this
  // path removal, they will be removed as well. Returns the extracted value on
  // success.
  // Note: If there is only one component in the path, use ExtractKey() instead.
  //
  // Example:
  //   Optional<Value> maybe_value = value.ExtractPath("foo.bar");
  Optional<Value> ExtractPath(StringPiece path);

  using dict_iterator_proxy = detail::dict_iterator_proxy;
  using const_dict_iterator_proxy = detail::const_dict_iterator_proxy;

  // |DictItems| returns a proxy object that exposes iterators to the underlying
  // dictionary. These are intended for iteration over all items in the
  // dictionary and are compatible with for-each loops and standard library
  // algorithms.
  //
  // Unlike with std::map, a range-for over the non-const version of DictItems()
  // will range over items of type pair<const std::string&, Value&>, so code of
  // the form
  //   for (auto kv : my_value.DictItems())
  //     Mutate(kv.second);
  // will actually alter |my_value| in place (if it isn't const).
  //
  // Note: These CHECK that type() is Type::DICTIONARY.
  dict_iterator_proxy DictItems();
  const_dict_iterator_proxy DictItems() const;

  // Returns the size of the dictionary, and if the dictionary is empty.
  // Note: These CHECK that type() is Type::DICTIONARY.
  size_t DictSize() const;
  bool DictEmpty() const;

  // Merge |dictionary| into this value. This is done recursively, i.e. any
  // sub-dictionaries will be merged as well. In case of key collisions, the
  // passed in dictionary takes precedence and data already present will be
  // replaced. Values within |dictionary| are deep-copied, so |dictionary| may
  // be freed any time after this call.
  // Note: This CHECKs that type() and dictionary->type() is Type::DICTIONARY.
  void MergeDictionary(const Value* dictionary);

  // DictionaryValue::From is the equivalent for std::unique_ptr conversions.
  bool GetAsDictionary(DictionaryValue** out_value);
  bool GetAsDictionary(const DictionaryValue** out_value) const;
  // Note: Do not add more types. See the file-level comment above for why.

  // Comparison operators so that Values can easily be used with standard
  // library algorithms and associative containers.
  CRBASE_EXPORT friend bool operator==(const Value& lhs, const Value& rhs);
  CRBASE_EXPORT friend bool operator!=(const Value& lhs, const Value& rhs);
  CRBASE_EXPORT friend bool operator<(const Value& lhs, const Value& rhs);
  CRBASE_EXPORT friend bool operator>(const Value& lhs, const Value& rhs);
  CRBASE_EXPORT friend bool operator<=(const Value& lhs, const Value& rhs);
  CRBASE_EXPORT friend bool operator>=(const Value& lhs, const Value& rhs);

  // Compares if two Value objects have equal contents.
  // DEPRECATED, use operator==(const Value& lhs, const Value& rhs) instead.
  // TODO(crbug.com/646113): Delete this and migrate callsites.
  bool Equals(const Value* other) const;

  // Estimates dynamic memory usage.
  // See base/trace_event/memory_usage_estimator.h for more info.
  size_t EstimateMemoryUsage() const;

 protected:
  // Special case for doubles, which are aligned to 8 bytes on some
  // 32-bit architectures. In this case, a simple declaration as a
  // double member would make the whole union 8 byte-aligned, which
  // would also force 4 bytes of wasted padding space before it in
  // the Value layout.
  //
  // To override this, store the value as an array of 32-bit integers, and
  // perform the appropriate bit casts when reading / writing to it.
  Type type_ = Type::NONE;

  union {
    bool bool_value_;
    int int_value_;
    DoubleStorage double_value_;
    std::string string_value_;
    BlobStorage binary_value_;
    DictStorage dict_;
    ListStorage list_;
  };

 private:
  friend class ValuesTest_SizeOfValue_Test;
  double AsDoubleInternal() const;
  void InternalMoveConstructFrom(Value&& that);
  void InternalCleanup();

  // NOTE: Using a movable reference here is done for performance (it avoids
  // creating + moving + destroying a temporary unique ptr).
  Value* SetKeyInternal(StringPiece key, std::unique_ptr<Value>&& val_ptr);
  Value* SetPathInternal(StringPiece path, std::unique_ptr<Value>&& value_ptr);
};

// DictionaryValue provides a key-value dictionary with (optional) "path"
// parsing for recursive access; see the comment at the top of the file. Keys
// are |std::string|s and should be UTF-8 encoded.
class CRBASE_EXPORT DictionaryValue : public Value {
 public:
  // Returns |value| if it is a dictionary, nullptr otherwise.
  static std::unique_ptr<DictionaryValue> From(std::unique_ptr<Value> value);

  DictionaryValue();
  explicit DictionaryValue(const DictStorage& in_dict);
  explicit DictionaryValue(DictStorage&& in_dict) noexcept;

  // Returns the number of Values in this dictionary.
  size_t size() const { return dict_.size(); }

  // Returns whether the dictionary is empty.
  bool empty() const { return dict_.empty(); }

  // Clears any current contents of this dictionary.
  void Clear();

  // Makes a copy of |this| but doesn't include empty dictionaries and lists in
  // the copy.  This never returns NULL, even if |this| itself is empty.
  std::unique_ptr<DictionaryValue> DeepCopyWithoutEmptyChildren() const;

  // Swaps contents with the |other| dictionary.
  void Swap(DictionaryValue* other);
};

// This type of Value represents a list of other Value values.
class CRBASE_EXPORT ListValue : public Value {
 public:
  // Returns |value| if it is a list, nullptr otherwise.
  static std::unique_ptr<ListValue> From(std::unique_ptr<Value> value);

  ListValue();
  explicit ListValue(Span<const Value> in_list);
  explicit ListValue(ListStorage&& in_list) noexcept;

  bool GetDictionary(size_t index, const DictionaryValue** out_value) const;
  bool GetDictionary(size_t index, DictionaryValue** out_value);
};

// This interface is implemented by classes that know how to serialize
// Value objects.
class CRBASE_EXPORT ValueSerializer {
 public:
  virtual ~ValueSerializer();

  virtual bool Serialize(const Value& root) = 0;
};

// This interface is implemented by classes that know how to deserialize Value
// objects.
class CRBASE_EXPORT ValueDeserializer {
 public:
  virtual ~ValueDeserializer();

  // This method deserializes the subclass-specific format into a Value object.
  // If the return value is non-NULL, the caller takes ownership of returned
  // Value. If the return value is NULL, and if error_code is non-NULL,
  // error_code will be set with the underlying error.
  // If |error_message| is non-null, it will be filled in with a formatted
  // error message including the location of the error if appropriate.
  virtual std::unique_ptr<Value> Deserialize(int* error_code,
                                             std::string* error_str) = 0;

  // The integer-valued error codes form four groups:
  //  - The value 0 means no error.
  //  - Values between 1 and 999 inclusive mean an error in the data (i.e.
  //    content). The bytes being deserialized are not in the right format.
  //  - Values 1000 and above mean an error in the metadata (i.e. context). The
  //    file could not be read, the network is down, etc.
  //  - Negative values are reserved.
  enum ErrorCode {
    kErrorCodeNoError = 0,
    // kErrorCodeInvalidFormat is a generic error code for "the data is not in
    // the right format". Subclasses of ValueDeserializer may return other
    // values for more specific errors.
    kErrorCodeInvalidFormat = 1,
    // kErrorCodeFirstMetadataError is the minimum value (inclusive) of the
    // range of metadata errors.
    kErrorCodeFirstMetadataError = 1000,
  };

  // The |error_code| argument can be one of the ErrorCode values, but it is
  // not restricted to only being 0, 1 or 1000. Subclasses of ValueDeserializer
  // can define their own error code values.
  static inline bool ErrorCodeIsDataError(int error_code) {
    return (kErrorCodeInvalidFormat <= error_code) &&
      (error_code < kErrorCodeFirstMetadataError);
  }
};

// Stream operator so Values can be used in assertion statements.  In order that
// gtest uses this operator to print readable output on test failures, we must
// override each specific type. Otherwise, the default template implementation
// is preferred over an upcast.
CRBASE_EXPORT std::ostream& operator<<(std::ostream& out, const Value& value);

CRBASE_EXPORT inline std::ostream& operator<<(std::ostream& out,
                                              const DictionaryValue& value) {
  return out << static_cast<const Value&>(value);
}

CRBASE_EXPORT inline std::ostream& operator<<(std::ostream& out,
                                              const ListValue& value) {
  return out << static_cast<const Value&>(value);
}

// Stream operator so that enum class Types can be used in log statements.
CRBASE_EXPORT std::ostream& operator<<(std::ostream& out,
                                       const Value::Type& type);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_VALUES_H_
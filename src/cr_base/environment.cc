// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/environment.h"

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
typedef struct IUnknown IUnknown;
#include <windows.h>
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <stdlib.h>
#endif

#include "cr_base/memory/ptr_util.h"
#include "cr_base/strings/string_piece.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/strings/utf_string_conversions.h"

namespace cr {

namespace {

class EnvironmentImpl : public Environment {
 public:
  bool GetVar(StringPiece variable_name, std::string* result) override {
    if (GetVarImpl(variable_name, result))
      return true;

    // Some commonly used variable names are uppercase while others
    // are lowercase, which is inconsistent. Let's try to be helpful
    // and look for a variable name with the reverse case.
    // I.e. HTTP_PROXY may be http_proxy for some users/systems.
    char first_char = variable_name[0];
    std::string alternate_case_var;
    if (IsAsciiLower(first_char))
      alternate_case_var = ToUpperASCII(variable_name);
    else if (IsAsciiUpper(first_char))
      alternate_case_var = ToLowerASCII(variable_name);
    else
      return false;
    return GetVarImpl(alternate_case_var, result);
  }

  bool SetVar(StringPiece variable_name,
              const std::string& new_value) override {
    return SetVarImpl(variable_name, new_value);
  }

  bool UnSetVar(StringPiece variable_name) override {
    return UnSetVarImpl(variable_name);
  }

 private:
  bool GetVarImpl(StringPiece variable_name, std::string* result) {
#if defined(MINI_CHROMIUM_OS_WIN)
    DWORD value_length =
        ::GetEnvironmentVariableW(
            UTF8ToWide(variable_name).c_str(), nullptr, 0);
    if (value_length == 0)
      return false;
    if (result) {
      std::unique_ptr<wchar_t[]> value(new wchar_t[value_length]);
      ::GetEnvironmentVariableW(UTF8ToWide(variable_name).c_str(), value.get(),
                                value_length);
      *result = WideToUTF8(value.get());
    }
    return true;
#elif defined(MINI_CHROMIUM_OS_POSIX)
    const char* env_value = getenv(variable_name.data());
    if (!env_value)
      return false;
    // Note that the variable may be defined but empty.
    if (result)
      *result = env_value;
    return true;
#endif
  }

  bool SetVarImpl(StringPiece variable_name, const std::string& new_value) {
#if defined(MINI_CHROMIUM_OS_WIN)
    // On success, a nonzero value is returned.
    return !!SetEnvironmentVariableW(UTF8ToWide(variable_name).c_str(),
                                     UTF8ToWide(new_value).c_str());
#elif defined(MINI_CHROMIUM_OS_POSIX)
    // On success, zero is returned.
    return !setenv(variable_name.data(), new_value.c_str(), 1);
#endif
  }

  bool UnSetVarImpl(StringPiece variable_name) {
#if defined(MINI_CHROMIUM_OS_WIN)
    // On success, a nonzero value is returned.
    return !!SetEnvironmentVariableW(UTF8ToWide(variable_name).c_str(), 
                                     nullptr);
#elif defined(MINI_CHROMIUM_OS_POSIX)
    // On success, zero is returned.
    return !unsetenv(variable_name.data());
#endif
  }
};


// Parses a null-terminated input string of an environment block. The key is
// placed into the given string, and the total length of the line, including
// the terminating null, is returned.
size_t ParseEnvLine(const NativeEnvironmentString::value_type* input,
  NativeEnvironmentString* key) {
  // Skip to the equals or end of the string, this is the key.
  size_t cur = 0;
  while (input[cur] && input[cur] != '=')
    cur++;
  *key = NativeEnvironmentString(&input[0], cur);

  // Now just skip to the end of the string.
  while (input[cur])
    cur++;
  return cur + 1;
}

}  // namespace

Environment::~Environment() = default;

// static
std::unique_ptr<Environment> Environment::Create() {
  return std::make_unique<EnvironmentImpl>();
}

bool Environment::HasVar(StringPiece variable_name) {
  return GetVar(variable_name, nullptr);
}

#if defined(MINI_CHROMIUM_OS_POSIX)

std::unique_ptr<char* []> AlterEnvironment(const char* const* const env,
                                           const EnvironmentMap& changes) {
  std::string value_storage;  // Holds concatenated null-terminated strings.
  std::vector<size_t> result_indices;  // Line indices into value_storage.

  // First build up all of the unchanged environment strings. These are
  // null-terminated of the form "key=value".
  std::string key;
  for (size_t i = 0; env[i]; i++) {
    size_t line_length = ParseEnvLine(env[i], &key);

    // Keep only values not specified in the change vector.
    auto found_change = changes.find(key);
    if (found_change == changes.end()) {
      result_indices.push_back(value_storage.size());
      value_storage.append(env[i], line_length);
    }
  }

  // Now append all modified and new values.
  for (const auto& i : changes) {
    if (!i.second.empty()) {
      result_indices.push_back(value_storage.size());
      value_storage.append(i.first);
      value_storage.push_back('=');
      value_storage.append(i.second);
      value_storage.push_back(0);
    }
  }

  size_t pointer_count_required =
      result_indices.size() + 1 +  // Null-terminated array of pointers.
      (value_storage.size() + sizeof(char*) - 1) / sizeof(char*);  // Buffer.
  std::unique_ptr<char*[]> result(new char*[pointer_count_required]);

  // The string storage goes after the array of pointers.
  char* storage_data =
      reinterpret_cast<char*>(&result.get()[result_indices.size() + 1]);
  if (!value_storage.empty())
    memcpy(storage_data, value_storage.data(), value_storage.size());

  // Fill array of pointers at the beginning of the result.
  for (size_t i = 0; i < result_indices.size(); i++)
    result[i] = &storage_data[result_indices[i]];
  result[result_indices.size()] = 0;  // Null terminator.

  return result;
}

#elif defined(MINI_CHROMIUM_OS_WIN)

NativeEnvironmentString AlterEnvironment(const wchar_t* env,
                                         const EnvironmentMap& changes) {
  NativeEnvironmentString result;

  // First build up all of the unchanged environment strings.
  const wchar_t* ptr = env;
  while (*ptr) {
    std::wstring key;
    size_t line_length = ParseEnvLine(ptr, &key);

    // Keep only values not specified in the change vector.
    if (changes.find(key) == changes.end()) {
      result.append(ptr, line_length);
    }
    ptr += line_length;
  }

  // Now append all modified and new values.
  for (const auto& i : changes) {
    // Windows environment blocks cannot handle keys or values with NULs.
    CR_CHECK(std::wstring::npos == i.first.find(L'\0'));
    CR_CHECK(std::wstring::npos == i.second.find(L'\0'));
    if (!i.second.empty()) {
      result += i.first;
      result.push_back('=');
      result += i.second;
      result.push_back('\0');
    }
  }

  // Add the terminating NUL.
  result.push_back('\0');
  return result;
}

#endif  // MINI_CHROMIUM_OS_POSIX

}  // namespace cr
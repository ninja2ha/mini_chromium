// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_ENVIRONMENT_H_
#define MINI_CHROMIUM_SRC_CRBASE_ENVIRONMENT_H_

#include <map>
#include <memory>
#include <string>

#include "cr_base/compiler_config.h"

#include "cr_base/base_export.h"
#include "cr_base/strings/string_piece.h"

namespace cr {

class CRBASE_EXPORT Environment {
 public:
  virtual ~Environment();

  // Returns the appropriate platform-specific instance.
  static std::unique_ptr<Environment> Create();

  // Gets an environment variable's value and stores it in |result|.
  // Returns false if the key is unset.
  virtual bool GetVar(StringPiece variable_name, std::string* result) = 0;

  // Syntactic sugar for GetVar(variable_name, nullptr);
  virtual bool HasVar(StringPiece variable_name);

  // Returns true on success, otherwise returns false. This method should not
  // be called in a multi-threaded process.
  virtual bool SetVar(StringPiece variable_name,
                      const std::string& new_value) = 0;

  // Returns true on success, otherwise returns false. This method should not
  // be called in a multi-threaded process.
  virtual bool UnSetVar(StringPiece variable_name) = 0;
};

#if defined(MINI_CHROMIUM_OS_WIN)
using NativeEnvironmentString = std::wstring;
#elif defined(MINI_CHROMIUM_OS_POSIX)
using NativeEnvironmentString = std::string;
#endif
using EnvironmentMap =
    std::map<NativeEnvironmentString, NativeEnvironmentString>;

#if defined(MINI_CHROMIUM_OS_POSIX)
// Returns a modified environment vector constructed from the given environment
// and the list of changes given in |changes|. Each key in the environment is
// matched against the first element of the pairs. In the event of a match, the
// value is replaced by the second of the pair, unless the second is empty, in
// which case the key-value is removed.
//
// This POSIX version takes and returns a POSIX-style environment block, which
// is a null-terminated list of pointers to null-terminated strings. The
// returned array will have appended to it the storage for the array itself so
// there is only one pointer to manage, but this means that you can't copy the
// array without keeping the original around.
CRBASE_EXPORT std::unique_ptr<char*[]> AlterEnvironment(
  const char* const* env,
  const EnvironmentMap& changes);
#elif defined(MINI_CHROMIUM_OS_WIN)
// Returns a modified environment vector constructed from the given environment
// and the list of changes given in |changes|. Each key in the environment is
// matched against the first element of the pairs. In the event of a match, the
// value is replaced by the second of the pair, unless the second is empty, in
// which case the key-value is removed.
//
// This Windows version takes and returns a Windows-style environment block,
// which is a string containing several null-terminated strings followed by an
// extra terminating null character. So, e.g., the environment A=1 B=2 is
// represented as L"A=1\0B=2\0\0".
CRBASE_EXPORT NativeEnvironmentString
AlterEnvironment(const wchar_t* env, const EnvironmentMap& changes);
#endif  // OS_*

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_ENVIRONMENT_H_
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This class works with command lines: building and parsing.
// Arguments with prefixes ('--', '-', and on Windows, '/') are switches.
// Switches will precede all other arguments without switch prefixes.
// Switches can optionally have values, delimited by '=', e.g., "-switch=value".
// If a switch is specified multiple times, only the last value is used.
// An argument of "--" will terminate switch parsing during initialization,
// interpreting subsequent tokens as non-switch arguments, regardless of prefix.

// There is a singleton read-only CommandLine that represents the command line
// that the current process was started with.  It must be initialized in main().

#ifndef MINI_CHROMIUM_SRC_CRBASE_COMMAND_LINE_H_
#define MINI_CHROMIUM_SRC_CRBASE_COMMAND_LINE_H_

#include <stddef.h>
#include <map>
#include <string>
#include <vector>

#include "cr_base/base_export.h"
#include "cr_base/strings/string_piece.h"
#include "cr_build/build_config.h"

namespace cr {

class FilePath;

class CRBASE_EXPORT CommandLine {
 public:
#if defined(MINI_CHROMIUM_OS_WIN)
  // The native command line string type.
  using StringType = std::wstring;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  using StringType = std::string;
#endif

  using CharType = StringType::value_type;
  using StringPieceType = cr::BasicStringPiece<CharType>;
  using StringVector = std::vector<StringType>;
  using SwitchMap = std::map<std::string, StringVector>;

  // A constructor for CommandLines that only carry switches and arguments.
  enum NoProgram { NO_PROGRAM };
  explicit CommandLine(NoProgram no_program);

  // Construct a new command line with |program| as argv[0].
  explicit CommandLine(const FilePath& program);

  // Construct a new command line from an argument list.
  CommandLine(int argc, const CharType* const* argv);
  explicit CommandLine(const StringVector& argv);

  CommandLine(const CommandLine& other);
  CommandLine& operator=(const CommandLine& other);

  ~CommandLine();

#if defined(MINI_CHROMIUM_OS_WIN)
  // By default this class will treat command-line arguments beginning with
  // slashes as switches on Windows, but not other platforms.
  //
  // If this behavior is inappropriate for your application, you can call this
  // function BEFORE initializing the current process' global command line
  // object and the behavior will be the same as Posix systems (only hyphens
  // begin switches, everything else will be an arg).
  static void set_slash_is_not_a_switch();

  // Normally when the CommandLine singleton is initialized it gets the command
  // line via the GetCommandLineW API and then uses the shell32 API
  // CommandLineToArgvW to parse the command line and convert it back to
  // argc and argv. If you don't want this dependency on shell32 and need
  // to honor the arguments passed in should use this function.
  // the |argv| is encoded by utf-8.
  static void InitUsingArgv(int argc, const char* const* argv);
#endif  // MINI_CHROMIUM_ OS_WIN

  // Initialize the current process CommandLine singleton. On Windows, ignores
  // its arguments (we instead parse GetCommandLineW() directly) because we
  // don't trust the CRT's parsing of the command line, but it still must be
  // called to set up the command line. Returns false if initialization has
  // already occurred, and true otherwise. Only the caller receiving a 'true'
  // return value should take responsibility for calling Reset.
  static bool Init(int argc, const char* const* argv);

  // Destroys the current process CommandLine singleton. This is necessary if
  // you want to reset the base library to its initial state (for example, in an
  // outer library that needs to be able to terminate, and be re-initialized).
  // If Init is called only once, as in main(), Reset() is not necessary.
  // Do not call this in tests. Use base::test::ScopedCommandLine instead.
  static void Reset();

  // Get the singleton CommandLine representing the current process's
  // command line. Note: returned value is mutable, but not thread safe;
  // only mutate if you know what you're doing!
  static CommandLine* ForCurrentProcess();

  // Returns true if the CommandLine has been initialized for the given process.
  static bool InitializedForCurrentProcess();

#if defined(MINI_CHROMIUM_OS_WIN)
  static CommandLine FromString(StringPieceType command_line);
#endif

  // Initialize from an argv vector.
  void InitFromArgv(int argc, const CharType* const* argv);
  void InitFromArgv(const StringVector& argv);

  enum SwitchKeyPrefixStyle {
    kKeyStyleNone,  //
    kKeyStyleSub,   // '-'
    kKeyStyleDSub,  // '--'
#if defined(MINI_CHROMIUM_OS_WIN)
    kKeyStyleSlash, // '/'
#endif
  };
  enum SwitchValueSeparatorStyle {
    kValueStyleNone,  //
    kValueStyleEqual, // '='
    kValueStyleMark,  // ':'
    kValueStyleSpace, // ' '
  };

  // Constructs and returns the represented command line string.
  // CAUTION! This should be avoided on POSIX because quoting behavior is
  // unclear.
  // CAUTION! If writing a command line to the Windows registry, use
  // GetCommandLineStringForShell() instead.
  StringType GetCommandLineString(
      SwitchKeyPrefixStyle key_style = kKeyStyleNone,
      SwitchValueSeparatorStyle separator_style = kValueStyleNone) const;

#if defined(MINI_CHROMIUM_OS_WIN)
  // Returns the represented command-line string. Allows the use of unsafe
  // Windows insert sequences like "%1". Only use this method if
  // GetCommandLineStringForShell() is not adequate AND the processor inserting
  // the arguments is known to do so securely (i.e., is not the Windows shell).
  // If in doubt, do not use.
  StringType GetCommandLineStringWithUnsafeInsertSequences(
      SwitchKeyPrefixStyle key_style = kKeyStyleNone,
      SwitchValueSeparatorStyle separator_style = kValueStyleNone) const;
#endif

  // Constructs and returns the represented arguments string.
  // CAUTION! This should be avoided on POSIX because quoting behavior is
  // unclear.
  StringType GetArgumentsString(
      SwitchKeyPrefixStyle key_style = kKeyStyleNone,
      SwitchValueSeparatorStyle separator_style = kValueStyleNone) const;

  // Returns the original command line string as a vector of strings.
  const StringVector& argv() const { return argv_; }

  // Get and Set the program part of the command line string (the first item).
  FilePath GetProgram() const;
  void SetProgram(const FilePath& program);

  // Returns true if this command line contains the given switch.
  // Switch names must be lowercase.
  // The second override provides an optimized version to avoid inlining codegen
  // at every callsite to find the length of the constant and construct a
  // StringPiece.
  bool HasSwitch(const StringPiece& switch_string) const;
  bool HasSwitch(const char switch_constant[]) const;

  // Returns the value associated with the given switch. If the switch has no
  // value or isn't present, this method returns the empty string.
  // Switch names must be lowercase.
  std::string GetSwitchValueASCII(const StringPiece& switch_string) const;
  FilePath GetSwitchValuePath(const StringPiece& switch_string) const;
  StringType GetSwitchValueNative(const StringPiece& switch_string) const;

  std::vector<FilePath> GetSwitchValueListPath(
      const StringPiece& switch_string) const;
  StringVector GetSwitchValueListNative(const StringPiece& switch_string) const;

  // Get a copy of all switches, along with their values.
  const SwitchMap& GetSwitches() const { return switches_; }

  // Append a switch [with optional value] to the command line.
  // Note: Switches will precede arguments regardless of appending order.
  void AppendSwitch(const std::string& switch_string);
  void AppendSwitchPath(const std::string& switch_string,
                        const FilePath& path);
  void AppendSwitchNative(const std::string& switch_string,
                          StringPieceType value);
  void AppendSwitchASCII(const std::string& switch_string,
                         const std::string& value);

  // Removes the switch that matches |switch_key_without_prefix|, regardless of
  // prefix and value. If no such switch is present, this has no effect.
  void RemoveSwitch(const cr::StringPiece switch_key_without_prefix);

  // Copy a set of switches (and any values) from another command line.
  // Commonly used when launching a subprocess.
  void CopySwitchesFrom(const CommandLine& source,
                        const char* const switches[],
                        size_t count);

  // Get the remaining arguments to the command.
  const StringVector& GetArgs() const;

  // Append an argument to the command line. Note that the argument is quoted
  // properly such that it is interpreted as one argument to the target command.
  // AppendArg is primarily for ASCII; non-ASCII input is interpreted as UTF-8.
  // Note: Switches will precede arguments regardless of appending order.
  void AppendArg(const std::string& value);
  void AppendArgPath(const FilePath& value);
  void AppendArgNative(const StringType& value);

  // Append the switches and arguments from another command line to this one.
  // If |include_program| is true, include |other|'s program as well.
  void AppendArguments(const CommandLine& other, bool include_program);

  // Insert a command before the current command.
  // Common for debuggers, like "gdb --args".
  void PrependWrapper(const StringType& wrapper);

#if defined(MINI_CHROMIUM_OS_WIN)
  // Initialize by parsing the given command line string.
  // The program name is assumed to be the first item in the string.
  void ParseFromString(StringPieceType command_line);
#endif

 private:
  // Disallow default constructor; a program name must be explicitly specified.
  CommandLine() = delete;
  // Allow the copy constructor. A common pattern is to copy of the current
  // process's command line and then add some flags to it. For example:
  //   CommandLine cl(*CommandLine::ForCurrentProcess());
  //   cl.AppendSwitch(...);

  // Append switches and arguments, keeping switches before arguments.
  void AppendSwitchesAndArguments(const StringVector& argv);

  // Internal version of GetArgumentsString to support allowing unsafe insert
  // sequences in rare cases (see
  // GetCommandLineStringWithUnsafeInsertSequences).
  StringType GetArgumentsStringInternal(
      bool allow_unsafe_insert_sequences, 
      SwitchKeyPrefixStyle key_style,
      SwitchValueSeparatorStyle separator_style) const;

  // The singleton CommandLine representing the current process's command line.
  static CommandLine* current_process_commandline_;

  // The argv array: { program, [(--|-|/)switch[=value]]*, [--], [argument]* }
  StringVector argv_;

  // Parsed-out switch keys and values.
  SwitchMap switches_;

  // The argument without switch array
  StringVector args_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_COMMAND_LINE_H_

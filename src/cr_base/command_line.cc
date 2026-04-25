// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/command_line.h"

#include <ostream>

#include "cr_base/containers/span.h"
#include "cr_base/files/file_path.h"
#include "cr_base/logging/logging.h"
#include "cr_base/stl_util.h"
#include "cr_base/strings/strcat.h"
#include "cr_base/strings/string_piece.h"
#include "cr_base/strings/string_split.h"
#include "cr_base/strings/string_tokenizer.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/strings/utf_string_conversions.h"
#include "cr_build/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"
#include <shellapi.h>
#endif  // defined(MINI_CHROMIUM_OS_WIN)

namespace cr {

CommandLine* CommandLine::current_process_commandline_ = nullptr;

namespace {

constexpr CommandLine::CharType kSwitchTerminator[] 
    = FILE_PATH_LITERAL("--");
constexpr CommandLine::CharType kSwitchValueSeparator[] 
    = FILE_PATH_LITERAL("=");

// Since we use a lazy match, make sure that longer versions (like "--") are
// listed before shorter versions (like "-") of similar prefixes.
// Warning: do not changing |kSwitchPrefixes| values order.
#if defined(MINI_CHROMIUM_OS_WIN)
// By putting slash last, we can control whether it is treaded as a switch
// value by changing the value of switch_prefix_count to be one less than
// the array size.
const CommandLine::StringPieceType kSwitchPrefixes[] = {L"--", L"-", L"/"};
#elif defined(MINI_CHROMIUM_OS_POSIX)
// Unixes don't use slash as a switch.
const CommandLine::StringPieceType kSwitchPrefixes[] = {"--", "-"};
#endif
size_t switch_prefix_count = cr::size(kSwitchPrefixes);

const CommandLine::CharType* kSwitchValueSeparators[] =
  { FILE_PATH_LITERAL("="), 
    FILE_PATH_LITERAL(":")};

const CommandLine::CharType* SwitchValueSeparatorStyleToString(
    CommandLine::SwitchValueSeparatorStyle style) {
  const CommandLine::CharType* valu_separator = nullptr;
  switch (style) {
  case CommandLine::kValueStyleEqual:
    valu_separator = kSwitchValueSeparators[0];
    break;
  case CommandLine::kValueStyleMark:
    valu_separator = kSwitchValueSeparators[1];
    break;
  case CommandLine::kValueStyleSpace:
    valu_separator = FILE_PATH_LITERAL(" ");
    break;
  default:
    valu_separator = kSwitchValueSeparators[0];
    break;
  }
  return valu_separator;
}

const CommandLine::CharType* SwitchKeyPrefixStyleToString(
    CommandLine::SwitchKeyPrefixStyle style) {
  const CommandLine::CharType* valu_separator = nullptr;
  switch (style) {
  case CommandLine::kKeyStyleDSub:
    valu_separator = kSwitchPrefixes[0].data();
    break;
  case CommandLine::kKeyStyleSub:
    valu_separator = kSwitchPrefixes[1].data();
    break;
#if defined(MINI_CHROMIUM_OS_WIN)
  case CommandLine::kKeyStyleSlash:
    valu_separator = kSwitchPrefixes[2].data();
    break;
#endif
  default:
    valu_separator = kSwitchPrefixes[0].data();
    break;
  }
  return valu_separator;
}

size_t GetSwitchPrefixLength(CommandLine::StringPieceType string) {
  for (size_t i = 0; i < switch_prefix_count; ++i) {
    CommandLine::StringType prefix(kSwitchPrefixes[i]);
    if (string.substr(0, prefix.length()) == prefix)
      return prefix.length();
  }
  return 0;
}

size_t FindSwitchValueSeparator(CommandLine::StringPieceType string) {
  for (const auto& i : kSwitchValueSeparators) {
    size_t pos = string.find(i);
    if (pos != CommandLine::StringType::npos)
      return pos;
  }
  return CommandLine::StringType::npos;
}

// Fills in |switch_string| and |switch_value| if |string| is a switch.
// This will preserve the input switch prefix in the output |switch_string|.
bool IsSwitch(const CommandLine::StringType& string,
              CommandLine::StringType* switch_string,
              CommandLine::StringType* switch_value,
              bool* has_separator,
              bool remove_key_prefix = false) {
  switch_string->clear();
  switch_value->clear();
  size_t prefix_length = GetSwitchPrefixLength(string);
  if (prefix_length == 0 || prefix_length == string.length())
    return false;

  const size_t equals_position = FindSwitchValueSeparator(string);
  *has_separator = (equals_position != CommandLine::StringType::npos);
  *switch_string = string.substr(
      remove_key_prefix ? prefix_length : 0, 
      remove_key_prefix ? equals_position - prefix_length : equals_position);
  if (equals_position != CommandLine::StringType::npos)
    *switch_value = string.substr(equals_position + 1);
  return true;
}

// Returns true iff |string| represents a switch with key
// |switch_key_without_prefix|, regardless of value.
bool IsSwitchWithKey(CommandLine::StringPieceType string,
                     CommandLine::StringPieceType switch_key_without_prefix) {
  size_t prefix_length = GetSwitchPrefixLength(string);
  if (prefix_length == 0 || prefix_length == string.length())
    return false;

  const size_t equals_position = FindSwitchValueSeparator(string);
  return string.substr(prefix_length, equals_position - prefix_length) ==
         switch_key_without_prefix;
}

#if defined(MINI_CHROMIUM_OS_WIN)
// Quote a string as necessary for CommandLineToArgvW compatibility *on
// Windows*.
std::wstring QuoteForCommandLineToArgvW(const std::wstring& arg,
                                        bool allow_unsafe_insert_sequences) {
  // Ensure that GetCommandLineString isn't used to generate command-line
  // strings for the Windows shell by checking for Windows insert sequences like
  // "%1". GetCommandLineStringForShell should be used instead to get a string
  // with the correct placeholder format for the shell.
  CR_DCHECK(arg.size() != 2 || arg[0] != L'%' || allow_unsafe_insert_sequences);

  // We follow the quoting rules of CommandLineToArgvW.
  // http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
  std::wstring quotable_chars(L" \\\"");
  if (arg.find_first_of(quotable_chars) == std::wstring::npos) {
    // No quoting necessary.
    return arg;
  }

  std::wstring out;
  out.push_back('"');
  for (size_t i = 0; i < arg.size(); ++i) {
    if (arg[i] == '\\') {
      // Find the extent of this run of backslashes.
      size_t start = i, end = start + 1;
      for (; end < arg.size() && arg[end] == '\\'; ++end) {}
      size_t backslash_count = end - start;

      // Backslashes are escapes only if the run is followed by a double quote.
      // Since we also will end the string with a double quote, we escape for
      // either a double quote or the end of the string.
      if (end == arg.size() || arg[end] == '"') {
        // To quote, we need to output 2x as many backslashes.
        backslash_count *= 2;
      }
      for (size_t j = 0; j < backslash_count; ++j)
        out.push_back('\\');

      // Advance i to one before the end to balance i++ in loop.
      i = end - 1;
    } else if (arg[i] == '"') {
      out.push_back('\\');
      out.push_back('"');
    } else {
      out.push_back(arg[i]);
    }
  }
  out.push_back('"');

  return out;
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

}  // namespace

CommandLine::CommandLine(NoProgram no_program)
    : argv_(1) {
}

CommandLine::CommandLine(const FilePath& program)
    : argv_(1) {
  SetProgram(program);
}

CommandLine::CommandLine(int argc, const CommandLine::CharType* const* argv)
    : argv_(1) {
  InitFromArgv(argc, argv);
}

CommandLine::CommandLine(const StringVector& argv)
    : argv_(1) {
  InitFromArgv(argv);
}

CommandLine::CommandLine(const CommandLine& other) = default;

CommandLine& CommandLine::operator=(const CommandLine& other) = default;

CommandLine::~CommandLine() = default;

#if defined(MINI_CHROMIUM_OS_WIN)
// static
void CommandLine::set_slash_is_not_a_switch() {
  // The last switch prefix should be slash, so adjust the size to skip it.
  CR_DCHECK(cr::MakeSpan(kSwitchPrefixes).back() == L"/")
      << "Error: Last switch prefix is not a slash.";
  switch_prefix_count = cr::size(kSwitchPrefixes) - 1;
}

// static
void CommandLine::InitUsingArgv(int argc, const char* const* argv) {
  CR_DCHECK(!current_process_commandline_);
  current_process_commandline_ = new CommandLine(NO_PROGRAM);
  // On Windows we need to convert the command line arguments to std::wstring.
  CommandLine::StringVector argv_vector;
  for (int i = 0; i < argc; ++i)
    argv_vector.push_back(UTF8ToWide(argv[i]));
  current_process_commandline_->InitFromArgv(argv_vector);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

// static
bool CommandLine::Init(int argc, const char* const* argv) {
  if (current_process_commandline_) {
    // If this is intentional, Reset() must be called first. If we are using
    // the shared build mode, we have to share a single object across multiple
    // shared libraries.
    return false;
  }

  current_process_commandline_ = new CommandLine(NO_PROGRAM);
#if defined(MINI_CHROMIUM_OS_WIN)
  current_process_commandline_->ParseFromString(::GetCommandLineW());
#elif defined(MINI_CHROMIUM_OS_POSIX)
  current_process_commandline_->InitFromArgv(argc, argv);
#else
#error Unsupported platform
#endif

  return true;
}

// static
void CommandLine::Reset() {
  CR_DCHECK(current_process_commandline_);
  delete current_process_commandline_;
  current_process_commandline_ = nullptr;
}

// static
CommandLine* CommandLine::ForCurrentProcess() {
  CR_DCHECK(current_process_commandline_);
  return current_process_commandline_;
}

// static
bool CommandLine::InitializedForCurrentProcess() {
  return !!current_process_commandline_;
}

#if defined(MINI_CHROMIUM_OS_WIN)
// static
CommandLine CommandLine::FromString(StringPieceType command_line) {
  CommandLine cmd(NO_PROGRAM);
  cmd.ParseFromString(command_line);
  return cmd;
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

void CommandLine::InitFromArgv(int argc,
                               const CommandLine::CharType* const* argv) {
  StringVector new_argv;
  for (int i = 0; i < argc; ++i)
    new_argv.push_back(argv[i]);
  InitFromArgv(new_argv);
}

void CommandLine::InitFromArgv(const StringVector& argv) {
  argv_ = StringVector(1);
  switches_.clear();
  SetProgram(argv.empty() ? FilePath() : FilePath(argv[0]));
  AppendSwitchesAndArguments(argv);
}

FilePath CommandLine::GetProgram() const {
  return FilePath(argv_[0]);
}

void CommandLine::SetProgram(const FilePath& program) {
#if defined(MINI_CHROMIUM_OS_WIN)
  argv_[0] = StringType(TrimWhitespace(program.value(), TRIM_ALL));
#elif defined(MINI_CHROMIUM_OS_POSIX)
  TrimWhitespaceASCII(program.value(), TRIM_ALL, &argv_[0]);
#else
#error Unsupported platform
#endif
}

// TODO: Writing a testing.
bool CommandLine::HasSwitch(const StringPiece& switch_string) const {
  CR_DCHECK(ToLowerASCII(switch_string) == switch_string);
  return Contains(switches_, std::string(switch_string));
}

bool CommandLine::HasSwitch(const char switch_constant[]) const {
  return HasSwitch(StringPiece(switch_constant));
}

std::string CommandLine::GetSwitchValueASCII(
    const StringPiece& switch_string) const {
  StringType value = GetSwitchValueNative(switch_string);
#if defined(MINI_CHROMIUM_OS_WIN)
  if (!IsStringASCII(cr::AsStringPiece16(value))) {
#elif defined(MINI_CHROMIUM_OS_POSIX)
  if (!IsStringASCII(value)) {
#endif
    CR_DLOG(Warning) 
        << "Value of switch (" << switch_string << ") must be ASCII.";
    return std::string();
  }
#if defined(MINI_CHROMIUM_OS_WIN)
  return WideToUTF8(value);
#elif defined(MINI_CHROMIUM_OS_POSIX)
  return value;
#endif
}

FilePath CommandLine::GetSwitchValuePath(
    const StringPiece& switch_string) const {
  return FilePath(GetSwitchValueNative(switch_string));
}

CommandLine::StringType CommandLine::GetSwitchValueNative(
    const StringPiece& switch_string) const {
  CR_DCHECK(ToLowerASCII(switch_string) == switch_string);
  auto result = switches_.find(std::string(switch_string));
  return result == switches_.end() ? StringType() : result->second.back();
}

std::vector<FilePath> CommandLine::GetSwitchValueListPath(
    const StringPiece& switch_string) const {
  CommandLine::StringVector values = GetSwitchValueListNative(switch_string);
  std::vector<FilePath> result(values.size());
  for (size_t i = 0; i < values.size(); i++)
    result[i] = FilePath(values[i]);
  return result;
}

CommandLine::StringVector CommandLine::GetSwitchValueListNative(
    const StringPiece& switch_string) const {
  CR_DCHECK(ToLowerASCII(switch_string) == switch_string);
  auto result = switches_.find(std::string(switch_string));
  return 
      result == switches_.end() ? CommandLine::StringVector() : result->second;
}

void CommandLine::AppendSwitch(const std::string& switch_string) {
  AppendSwitchNative(switch_string, StringType());
}

void CommandLine::AppendSwitchPath(const std::string& switch_string,
                                   const FilePath& path) {
  AppendSwitchNative(switch_string, path.value());
}

void CommandLine::AppendSwitchNative(const std::string& switch_string,
                                     CommandLine::StringPieceType value) {
#if defined(MINI_CHROMIUM_OS_WIN)
  const std::string switch_key = ToLowerASCII(switch_string);
  StringType combined_switch_string(UTF8ToWide(switch_key));
#elif defined(MINI_CHROMIUM_OS_POSIX)
  const std::string& switch_key = switch_string;
  StringType combined_switch_string(switch_key);
#endif
  size_t prefix_length = GetSwitchPrefixLength(combined_switch_string);

  // insert or assign
  {
    const std::string key = switch_key.substr(prefix_length);
    auto it = switches_.find(key);
    if (it != switches_.end()) {
      // already have
      if(!value.empty()) {
        it->second.push_back(StringType(value));
      }
    } else {
      // key not exist.
      std::vector<StringType> val_list(value.empty() ? 0 : 1);
      if (!value.empty())
        val_list[0] = StringType(value);

      auto insertion = switches_.insert(std::make_pair(key, val_list));
      if (!insertion.second)
        insertion.first->second = std::move(val_list);
    }
  }

  // Preserve existing switch prefixes in |argv_|; only append one if necessary.
  if (prefix_length == 0) {
    combined_switch_string.insert(0, kSwitchPrefixes[0].data(),
                                  kSwitchPrefixes[0].size());
  }
  if (!value.empty()) {
    cr::StrAppend(&combined_switch_string, { kSwitchValueSeparator, value});
  }

  // Append the switch.
  argv_.push_back(std::move(combined_switch_string));
}

void CommandLine::AppendSwitchASCII(const std::string& switch_string,
                                    const std::string& value_string) {
#if defined(MINI_CHROMIUM_OS_WIN)
  AppendSwitchNative(switch_string, UTF8ToWide(value_string));
#elif defined(MINI_CHROMIUM_OS_POSIX)
  AppendSwitchNative(switch_string, value_string);
#else
#error Unsupported platform
#endif
}

void CommandLine::RemoveSwitch(cr::StringPiece switch_key_without_prefix) {
#if defined(MINI_CHROMIUM_OS_WIN)
  StringType switch_key_native = UTF8ToWide(switch_key_without_prefix);
#elif defined(MINI_CHROMIUM_OS_POSIX)
  StringType switch_key_native(switch_key_without_prefix);
#endif

  CR_DCHECK(ToLowerASCII(switch_key_without_prefix) == switch_key_without_prefix);
  CR_DCHECK(0u == GetSwitchPrefixLength(switch_key_native));
  auto it = switches_.find(std::string(switch_key_without_prefix));
  if (it == switches_.end())
    return;
  switches_.erase(it);
  // Also erase from the switches section of |argv_| 
  auto argv_switches_begin = argv_.begin() + 1;
  auto argv_switches_end = argv_.end();
  CR_DCHECK(argv_switches_begin <= argv_switches_end);
  CR_DCHECK(argv_switches_end <= argv_.end());
  auto expell = std::remove_if(argv_switches_begin, argv_switches_end,
                               [&switch_key_native](const StringType& arg) {
                                 return IsSwitchWithKey(arg, switch_key_native);
                               });
  if (expell == argv_switches_end) {
    CR_NOTREACHED();
    return;
  }

  argv_.erase(expell, argv_switches_end);
}

void CommandLine::CopySwitchesFrom(const CommandLine& source,
                                   const char* const switches[],
                                   size_t count) {
  for (size_t i = 0; i < count; ++i) {
    if (source.HasSwitch(switches[i]))
      AppendSwitchNative(switches[i], source.GetSwitchValueNative(switches[i]));
  }
}

const CommandLine::StringVector& CommandLine::GetArgs() const {
  return args_;
}

void CommandLine::AppendArg(const std::string& value) {
#if defined(MINI_CHROMIUM_OS_WIN)
  CR_DCHECK(IsStringUTF8(value));
  AppendArgNative(UTF8ToWide(value));
#elif defined(MINI_CHROMIUM_OS_POSIX)
  AppendArgNative(value);
#else
#error Unsupported platform
#endif
}

void CommandLine::AppendArgPath(const FilePath& path) {
  AppendArgNative(path.value());
}

void CommandLine::AppendArgNative(const CommandLine::StringType& value) {
  argv_.push_back(value);
  args_.push_back(value);
}

void CommandLine::AppendArguments(const CommandLine& other,
                                  bool include_program) {
  if (include_program)
    SetProgram(other.GetProgram());
  AppendSwitchesAndArguments(other.argv());
}

void CommandLine::PrependWrapper(const CommandLine::StringType& wrapper) {
  if (wrapper.empty())
    return;
  // Split the wrapper command based on whitespace (with quoting).
  using CommandLineTokenizer =
      StringTokenizerT<StringType, StringType::const_iterator>;
  CommandLineTokenizer tokenizer(wrapper, FILE_PATH_LITERAL(" "));
  tokenizer.set_quote_chars(FILE_PATH_LITERAL("'\""));
  std::vector<StringType> wrapper_argv;
  while (tokenizer.GetNext())
    wrapper_argv.emplace_back(tokenizer.token());

  // Prepend the wrapper.
  argv_.insert(argv_.begin(), wrapper_argv.begin(), wrapper_argv.end());
}

#if defined(MINI_CHROMIUM_OS_WIN)
void CommandLine::ParseFromString(StringPieceType command_line) {
  command_line = TrimWhitespace(command_line, TRIM_ALL);
  if (command_line.empty())
    return;

  int num_args = 0;
  wchar_t** args = NULL;
  // When calling CommandLineToArgvW, use the apiset if available.
  // Doing so will bypass loading shell32.dll on Win8+.
  ///HMODULE downlevel_shell32_dll =
  ///    ::LoadLibraryExW(L"api-ms-win-downlevel-shell32-l1-1-0.dll", nullptr,
  ///                     LOAD_LIBRARY_SEARCH_SYSTEM32);
  ///if (downlevel_shell32_dll) {
  ///  auto command_line_to_argv_w_proc =
  ///      reinterpret_cast<decltype(::CommandLineToArgvW)*>(
  ///          ::GetProcAddress(downlevel_shell32_dll, "CommandLineToArgvW"));
  ///  if (command_line_to_argv_w_proc)
  ///    args = command_line_to_argv_w_proc(command_line.data(), &num_args);
  ///} else {
    // Since the apiset is not available, allow the delayload of shell32.dll
    // to take place.
    args = ::CommandLineToArgvW(command_line.data(), &num_args);
  ///}

  CR_DPLOG_IF(Fatal, !args) << "CommandLineToArgvW failed on command line: "
                            << command_line;
  StringVector argv(args, args + num_args);
  InitFromArgv(argv);
  LocalFree(args);

  ///if (downlevel_shell32_dll)
  ///  ::FreeLibrary(downlevel_shell32_dll);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

void CommandLine::AppendSwitchesAndArguments(
    const CommandLine::StringVector& argv) {
  bool parse_switches = true;
  CommandLine::StringType last_switch_string;
  for (size_t i = 1; i < argv.size(); ++i) {
    CommandLine::StringType arg = argv[i];
#if defined(MINI_CHROMIUM_OS_WIN)
    arg = CommandLine::StringType(TrimWhitespace(arg, TRIM_ALL));
#elif defined(MINI_CHROMIUM_OS_POSIX)
    TrimWhitespaceASCII(arg, TRIM_ALL, &arg);
#endif

    bool has_separator;
    CommandLine::StringType switch_string;
    CommandLine::StringType switch_value;
    parse_switches &= (arg != kSwitchTerminator);
    if (parse_switches && 
        IsSwitch(arg, &switch_string, &switch_value, &has_separator)) {
      if (!last_switch_string.empty()) {
#if defined(MINI_CHROMIUM_OS_WIN)
        AppendSwitchNative(WideToUTF8(last_switch_string), 
                           CommandLine::StringType());
#elif defined(MINI_CHROMIUM_OS_POSIX)
        AppendSwitchNative(last_switch_string, CommandLine::StringType());
#endif
        last_switch_string.clear();
      }

      if (switch_value.empty() && !has_separator) {
        last_switch_string = switch_string;
        continue ;
      }
#if defined(MINI_CHROMIUM_OS_WIN)
      AppendSwitchNative(WideToUTF8(switch_string), switch_value);
#elif defined(MINI_CHROMIUM_OS_POSIX)
      AppendSwitchNative(switch_string, switch_value);
#else
#error Unsupported platform
#endif
    } else {
      if (!last_switch_string.empty()) {
#if defined(MINI_CHROMIUM_OS_WIN)
        AppendSwitchNative(WideToUTF8(last_switch_string), arg);
#elif defined(MINI_CHROMIUM_OS_POSIX)
        AppendSwitchNative(last_switch_string, arg);
#endif
        last_switch_string.clear();
      } else { 
        AppendArgNative(arg);
      }
    }
  }

  if (!last_switch_string.empty()) {
#if defined(MINI_CHROMIUM_OS_WIN)
    AppendSwitchNative(WideToUTF8(last_switch_string),
                       CommandLine::StringType());
#elif defined(MINI_CHROMIUM_OS_POSIX)
    AppendSwitchNative(last_switch_string, CommandLine::StringType());
#endif
  }
}

CommandLine::StringType CommandLine::GetArgumentsStringInternal(
    bool allow_unsafe_insert_sequences,
    SwitchKeyPrefixStyle key_style,
    SwitchValueSeparatorStyle separator_style) const {
  auto key_prefix = SwitchKeyPrefixStyleToString(key_style);
  auto value_separator = SwitchValueSeparatorStyleToString(separator_style);
  CR_DCHECK(key_prefix);
  CR_DCHECK(value_separator);

  StringType params;
  // Append switches and arguments.
  bool parse_switches = true;
  bool last_empty_switch = false;
  for (size_t i = 1; i < argv_.size(); ++i) {
    StringType arg = argv_[i];
    StringType switch_string;
    StringType switch_value;
    bool has_separator;
    parse_switches &= arg != kSwitchTerminator;

    if (parse_switches && IsSwitch(arg, &switch_string, &switch_value, 
                                   &has_separator, 
                                   key_style != kKeyStyleNone)) {
      if (i > 1)
        params.append(FILE_PATH_LITERAL(" "));

      if (key_style != kKeyStyleNone)
        params.append(key_prefix);
      params.append(switch_string);
      if (!switch_value.empty()) {
#if defined(MINI_CHROMIUM_OS_WIN)
        switch_value = QuoteForCommandLineToArgvW(
            switch_value, allow_unsafe_insert_sequences);
#endif
        params.append(value_separator + switch_value);
      } else {
        last_empty_switch = true;
      }
    } else {
      if (last_empty_switch) {
        last_empty_switch = false;
        params.append(value_separator);
      }

      if (i > 1)
        params.append(FILE_PATH_LITERAL(" "));

#if defined(MINI_CHROMIUM_OS_WIN)
      arg = QuoteForCommandLineToArgvW(arg, allow_unsafe_insert_sequences);
#endif
      params.append(arg);
    }
  }

  return params;
}

CommandLine::StringType CommandLine::GetCommandLineString(
    SwitchKeyPrefixStyle key_style,
    SwitchValueSeparatorStyle separator_style) const {
  StringType string(argv_[0]);
#if defined(MINI_CHROMIUM_OS_WIN)
  string = QuoteForCommandLineToArgvW(string,
                                      /*allow_unsafe_insert_sequences=*/false);
#endif
  StringType params(GetArgumentsString(key_style, separator_style));
  if (!params.empty()) {
    string.append(FILE_PATH_LITERAL(" "));
    string.append(params);
  }
  return string;
}

#if defined(MINI_CHROMIUM_OS_WIN)
CommandLine::StringType
CommandLine::GetCommandLineStringWithUnsafeInsertSequences(
    SwitchKeyPrefixStyle key_style,
     SwitchValueSeparatorStyle separator_style) const {
  StringType string(argv_[0]);
  string = QuoteForCommandLineToArgvW(string,
                                      /*allow_unsafe_insert_sequences=*/true);
  StringType params(
      GetArgumentsStringInternal(
          /*allow_unsafe_insert_sequences=*/true, key_style, separator_style));
  if (!params.empty()) {
    string.append(FILE_PATH_LITERAL(" "));
    string.append(params);
  }
  return string;
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

CommandLine::StringType CommandLine::GetArgumentsString(
    SwitchKeyPrefixStyle key_style,
    SwitchValueSeparatorStyle separator_style) const {
  return GetArgumentsStringInternal(
    /*allow_unsafe_insert_sequences=*/false, key_style, separator_style);
}

}  // namespace cr

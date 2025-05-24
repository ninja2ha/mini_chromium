// Copyright (c) 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <fstream>
#include <iostream>
#include <map>

#include "crbase/at_exit.h"
#include "crbase/functional/callback.h"
#include "crbase/functional/bind.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/string_util.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/strings/sys_string_conversions.h"
#include "crbase/files/file_path.h"
#include "crbase/files/file_util.h"
#include "crbase/files/file_enumerator.h"
#include "crbase/path_service.h"
#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <io.h>
#include "crbase/win/msvc_import_libs.h"
#endif

#include "examples/common/logging_initializtion.h"

namespace {

using FileNameToIncludePath = std::map<std::string, std::string>;

bool SkiaCheckPath(const cr::FilePath& skia_path, 
                   cr::FilePath& include_path,
                   cr::FilePath& src_path) {
  include_path = skia_path.Append(FILE_PATH_LITERAL("include"));
  src_path = skia_path.Append(FILE_PATH_LITERAL("src"));
  if (!cr::DirectoryExists(include_path) || !cr::DirectoryExists(src_path)) {
    include_path.clear();
    src_path.clear();
    return false;
  }
  return true;
}

//
// == e.g ==
//  |root|:        c:\\skia-chrome-m61
//  |file|:        c:\\skia-chrome-m61\\include\\core\\SkTypes.h
//  [posix_style]: true
//  [return]:      include/core/SkTypes.h
//
std::string SkiaClacPathIncludeFile(const cr::FilePath& root, 
                                    const cr::FilePath& file,
                                    bool posix_style) {
  cr::FilePath::StringType include_file 
      = root.value().length() + file.value().c_str() + 1;
#if defined(MINI_CHROMIUM_OS_WIN)
  if (posix_style)
    cr::ReplaceChars(include_file, L"\\", L"/", &include_file);
  return cr::UTF16ToUTF8(include_file);
#else
  return include_file;
#endif
}

//
// == sample ==
// |code_line|                   |Return|
//
// #include "foo1.h"          => StringPiece(10, 6) foo1.h
// #    include "foo2.h"      => StringPiece(14, 6) foo2.h
//     #include "foo3.h"      => StringPiece(14, 6) foo3.h
// #include    "foo4.h"       => StringPiece(13, 6) foo4.h
// #include "foo5.h" // xx    => StringPiece(10, 6) foo5.h
// #include "foo6.h" /* xx */ => StringPiece(10, 6) foo6.h
// // #include "foo7.h"       => StringPiece(0, 0)
// #include <foo1.h>          => StringPiece(10, 6) foo1.h
//
cr::StringPiece SkiaCppGetIncludeFile(cr::StringPiece code_line) {
  auto getStartPos = [](cr::StringPiece& code, cr::StringPiece pattern) 
      -> size_t {
    for (size_t i = 0; i < code.length(); i++) {
      if (code[i] == ' ' || code[i] == '\t')
        continue;
      return cr::StartsWith(
          cr::StringPiece(code.begin() + i, code.length() - i), 
          pattern,
          cr::CompareCase::SENSITIVE) ? i : cr::StringPiece::npos;
    }
    return cr::StringPiece::npos;
  };

  // Finding first non space char.
  size_t p = getStartPos(code_line, "#");
  if (p == cr::StringPiece::npos)
    return cr::StringPiece();

  const char* begin = code_line.begin() + p + 1;
  if (begin >= code_line.end())
    return cr::StringPiece();
  size_t remain_len = code_line.length() - p - 1;

  p = getStartPos(cr::StringPiece(begin, remain_len), "include");
  if (p == cr::StringPiece::npos)
    return cr::StringPiece();

  begin = begin + p + 7;
  if (begin >= code_line.end())
    return cr::StringPiece();
  remain_len = remain_len - p - 7;

  static const char* pattern1 = "\"";
  static const char* pattern2 = ">";

  const char* next_pattern = pattern1;
  p = getStartPos(cr::StringPiece(begin, remain_len), "\"");
  if (p == cr::StringPiece::npos) {
    p = getStartPos(cr::StringPiece(begin, remain_len), "<");
    if (p == cr::StringPiece::npos)
      return cr::StringPiece();
    next_pattern = pattern2;
  }

  begin = begin + p + 1;
  remain_len = remain_len - p - 1;

  p = cr::StringPiece(begin, remain_len).find(next_pattern);
  if (p == cr::StringPiece::npos || p == 0) {
    return cr::StringPiece();
  }

  return cr::StringPiece(begin, p);
}

bool SkiaIsCppFile(const cr::FilePath& file) {
  static const cr::FilePath::CharType* ext[] = { 
      FILE_PATH_LITERAL(".s"),
      FILE_PATH_LITERAL(".inc"),
      FILE_PATH_LITERAL(".include"),
      FILE_PATH_LITERAL(".fp"),
      FILE_PATH_LITERAL(".h"),
      FILE_PATH_LITERAL(".hpp"),
      FILE_PATH_LITERAL(".c"),
      FILE_PATH_LITERAL(".cc"),
      FILE_PATH_LITERAL(".cpp") };
  const auto& file_value = file.value();
  for (const auto i : ext)
    if (cr::EndsWith(file_value, i, cr::CompareCase::INSENSITIVE_ASCII)) 
      return true;
  return false;
}

void SkiaEnumerateDirectory(
    const cr::FilePath& root,
    const cr::FilePath& dir,
    cr::RepeatingCallback<
        bool(const cr::FilePath&, const cr::FilePath&, void* ud)> cb,
    void* ud) {

  cr::FileEnumerator file_enumerator(dir, true, cr::FileEnumerator::FILES);
  for (;;) {
    cr::FilePath file = file_enumerator.Next();
    if (file.empty())
      break;

    if (!SkiaIsCppFile(file))
      continue;

    if (!cb.Run(root, file, ud))
      break;
  }
}

int SkiaAddSourceTree(const cr::FilePath& root,
                      const cr::FilePath& include_path,
                      FileNameToIncludePath& include_map) {
  SkiaEnumerateDirectory(root, include_path, cr::BindRepeating([](
      const cr::FilePath& root, const cr::FilePath& file, void* ud) -> bool {
    FileNameToIncludePath& include_map = *(FileNameToIncludePath*)ud;
    std::string include_file = SkiaClacPathIncludeFile(root, file, true);
    std::string file_name = include_file.substr(include_file.rfind('/') + 1);
    if (file_name.empty()) {
      CR_DCHECK(false);
      return false;
    }

    auto it = include_map.find(file_name);
    if (it == include_map.end())
      include_map[file_name] = include_file;
    else
      CR_LOG(Warning) << "Found a same name file. skiped..";
    return true;
  }), &include_map);
  return 0;
}

int SkiaFormatIncludes(const cr::FilePath& root,
                       const cr::FilePath& src_path, 
                       const FileNameToIncludePath& include_map,
                       const cr::FilePath& out_root) {
  static cr::FilePath g_out_root;
  g_out_root = out_root;
  SkiaEnumerateDirectory(root, src_path, cr::BindRepeating([](
      const cr::FilePath& root, const cr::FilePath& file, void* ud) -> bool {
    const FileNameToIncludePath& include_map = 
        *(const FileNameToIncludePath*)ud;
    
    std::string relative_file = SkiaClacPathIncludeFile(root, file, false);
    cr::FilePath output_file = g_out_root.AppendASCII(relative_file);
    cr::FilePath output_dir = output_file.DirName();
    if (!cr::PathExists(output_dir))
      cr::CreateDirectory(output_dir);

    cr::File output(output_file,
                    cr::File::FLAG_CREATE_ALWAYS | 
                    cr::File::FLAG_WRITE);
#if defined(MINI_CHROMIUM_OS_WIN)
    std::ifstream input(cr::SysWideToNativeMB(file.value()), std::ios::in);
#elif defined(MINI_CHROMIUM_OS_POSIX)
    std::ifstream input(file.value(), std::ios::in);
#endif
    if (!input.is_open()) {
      CR_LOG(Error) << "Failed to read file." << file;
      return true;
    }

    if (!output.IsValid()) {
      CR_LOG(Error) << "Failed to write file." << output_file
                    << " Error:" << output.error_details();
      return true;
    }

    std::string code_line;
    int offset = 0;
    while (std::getline(input, code_line)) {
      cr::StringPiece cl_view(code_line);
      cr::StringPiece if_view = SkiaCppGetIncludeFile(cl_view);
      if (!if_view.empty()) {
        std::string include;
        size_t p = if_view.find_last_of("\\/");
        if (p == if_view.npos)
          include = if_view.as_string();
        else
          include = if_view.substr(p + 1).as_string();

        auto it = include_map.find(include);
        if (it != include_map.end()) {
          include = 
              cl_view.substr(0, if_view.begin() - cl_view.begin()).as_string();
          include += it->second;
          include += 
              cl_view.substr(if_view.end() - cl_view.begin()).as_string();
          code_line = std::move(include);
        }
      }
      code_line += "\n";
      int r = output.Write(
          offset, code_line.data(), static_cast<int>(code_line.size()));
      if (r < 0) {
        CR_LOG(Error) << "File write failed." << output_file;
        return true;
      }
      offset += r;
    }
    return true;
  }), const_cast<FileNameToIncludePath*>(&include_map));
  return 0;
}

}  // namesapce

int main3(int argc, char* argv[]) {
  example::InitLogging();
  cr::AtExitManager at_exit;

  std::cout << "This tool is used to format include file in skia source."
            << std::endl
            << "e.g: format the code from #include \"SkTypes.h\" to "
                                         "#include \"include/core/SkTypes.h\""
            << std::endl;
  std::string skia_path;
  std::cout << "now, typing skia path(e.g. E:\\skia-chrome-m61):";
  std::getline(std::cin, skia_path);

  cr::FilePath root;
  root = root.FromUTF16Unsafe(cr::SysNativeMBToWide(skia_path));

  cr::FilePath include_path, src_path;
  if (!SkiaCheckPath(root, include_path, src_path))
    return -1;

  cr::FilePath out_root;
  cr::PathService::Get(cr::DIR_EXE, &out_root);
  out_root = out_root.Append(FILE_PATH_LITERAL("skia"));
  if (cr::PathExists(out_root))
    cr::DeleteFile(out_root, true);

  FileNameToIncludePath include_map;
  SkiaAddSourceTree(root, include_path, include_map);
  SkiaFormatIncludes(root, include_path, include_map, out_root);

  SkiaAddSourceTree(root, src_path, include_map);
  SkiaFormatIncludes(root, src_path, include_map, out_root);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

static const char* skia_src[] = {
  "$_src/sksl/SkSLCFGGenerator.cpp",
  "$_src/sksl/SkSLCompiler.cpp",
  "$_src/sksl/SkSLIRGenerator.cpp",
  "$_src/sksl/SkSLParser.cpp",
  "$_src/sksl/SkSLCPPCodeGenerator.cpp",
  "$_src/sksl/SkSLHCodeGenerator.cpp",
  "$_src/sksl/SkSLGLSLCodeGenerator.cpp",
  "$_src/sksl/SkSLSPIRVCodeGenerator.cpp",
  "$_src/sksl/SkSLString.cpp",
  "$_src/sksl/SkSLUtil.cpp",
  "$_src/sksl/lex.layout.cpp",

};

int main(int argc, char* argv) {
  example::InitLogging();
  cr::AtExitManager at_exit;

  std::string out;
  for (size_t i = 0; i < cr::size(skia_src); i++) {
    cr::StringPiece cur = skia_src[i];
    if (!cr::EndsWith(cur, ".cpp", cr::CompareCase::SENSITIVE))
      continue;

    size_t p = cur.find_last_of('/');
    if (!out.empty())  out += " ";
    out += "\"";
    out += cur.substr(p + 1).as_string();
    out += "\"";
  }
  
  cr::FilePath out_file;
  cr::PathService::Get(cr::DIR_EXE, &out_file);
  out_file = out_file.AppendASCII("out.txt");

  cr::File file(out_file, cr::File::FLAG_CREATE_ALWAYS | cr::File::FLAG_WRITE);
  file.Write(0, out.data(), out.length());
  return 0;
}
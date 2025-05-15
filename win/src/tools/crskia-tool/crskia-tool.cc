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
#include "crbase/win/msvc_import_libs.h"
#include "crbase/path_service.h"
#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <io.h>
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

std::string SkiaGetIncludeFile(const cr::FilePath& root, 
                               const cr::FilePath& file,
                               bool replace_sep) {
  cr::FilePath::StringType include_file 
      = root.value().length() + file.value().c_str() + 1;
#if defined(MINI_CHROMIUM_OS_WIN)
  if (replace_sep) 
    cr::ReplaceChars(include_file, L"\\", L"/", &include_file);
  return cr::UTF16ToUTF8(include_file);
#else
  return include_file;
#endif
}

bool SkiaIsCppFile(const cr::FilePath& file) {
  static const cr::FilePath::CharType* ext[] = { 
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
    std::string include_file = SkiaGetIncludeFile(root, file, true);
    std::string file_name = "\"";
    file_name += include_file.substr(include_file.rfind('/') + 1);
    file_name += "\"";
    if (file_name.size() <= 2) {
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
    
    std::string include_file = SkiaGetIncludeFile(root, file, false);
    cr::FilePath out_file_path = g_out_root.AppendASCII(include_file);
    cr::FilePath out_dir = out_file_path.DirName();
    if (!cr::DirectoryExists(out_dir))
      cr::CreateDirectory(out_dir);

    cr::File outfile(out_file_path,
                     cr::File::FLAG_CREATE_ALWAYS | 
                     cr::File::FLAG_WRITE);
#if defined(MINI_CHROMIUM_OS_WIN)
    std::ifstream istream(cr::SysWideToNativeMB(file.value()), std::ios::in);
#elif defined(MINI_CHROMIUM_OS_POSIX)
    std::ifstream istream(file.value(), std::ios::in);
#endif
    if (!istream.is_open()) {
      CR_LOG(Error) << "Failed to read file." << file;
      return true;
    }

    if (!outfile.IsValid()) {
      CR_LOG(Error) << "Failed to write file." << out_file_path
                    << " Error:" <<  outfile.error_details();
      return true;
    }

    std::string codeline;
    int offset = 0;
    while (std::getline(istream, codeline)) {
      static cr::StringPiece pattern = "#include \"";
      if (cr::StartsWith(codeline, pattern, cr::CompareCase::SENSITIVE)) {
         std::string include = codeline.substr(pattern.length() - 1,
                                               codeline.rfind('\"'));
         auto p = include.rfind("/");
         if (p != std::string::npos) {
           include = include.substr(p);
           include[0] = '\"';
         }
         auto it = include_map.find(include);
         if (it != include_map.end()) {
           codeline = "#include \"";
           codeline += it->second;
           codeline += "\"";
         }
      }
      codeline += "\n";
      int r = outfile.Write(offset, codeline.data(), 
                            static_cast<int>(codeline.size()));
      if (r < 0) {
        CR_LOG(Error) << "File write failed." << out_file_path;
        return true;
      }
      offset += r;
    }
    return true;
  }), const_cast<FileNameToIncludePath*>(&include_map));
  return 0;
}

}  // namesapce

int main(int argc, char* argv[]) {
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
  if (cr::DirectoryExists(out_root))
    cr::DeleteFile(out_root, true);

  FileNameToIncludePath include_map;
  SkiaAddSourceTree(root, include_path, include_map);
  SkiaFormatIncludes(root, include_path, include_map, out_root);

  SkiaAddSourceTree(root, src_path, include_map);
  SkiaFormatIncludes(root, src_path, include_map, out_root);
  return 0;
}

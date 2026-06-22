// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/files/file_util.h"

#include "cr_base/compiler_config.h"

#include "cr_base/numerics/safe_math.h"
#include "cr_base/strings/stringprintf.h"
#include "cr_base/strings/strcat.h"
#include "cr_base/files/file_enumerator.h"
#include "cr_base/files/file.h"

namespace cr {

int64_t ComputeDirectorySize(const FilePath& root_path) {
  int64_t running_size = 0;
  FileEnumerator file_iter(root_path, true, FileEnumerator::FILES);
  while (!file_iter.Next().empty())
    running_size += file_iter.GetInfo().GetSize();
  return running_size;
}

bool ReadFileToString(const FilePath& path, std::string* contents) {
  return ReadFileToStringWithMaxSize(path, contents,
                                     std::numeric_limits<size_t>::max());
}

bool ReadFileToStringWithMaxSize(const FilePath& path,
                                 std::string* contents,
                                 size_t max_size) {
  if (contents)
    contents->clear();

  // Many files have incorrect size (proc files etc). Hence, the file is read
  // sequentially as opposed to a one-shot read, using file size as a hint for
  // chunk size if available.
  constexpr int64_t kDefaultChunkSize = 1 << 16;
  int64_t chunk_size = kDefaultChunkSize - 1;

  int64_t file_size = 0;
  if (GetFileSize(path, &file_size) && file_size > 0) {
    // We need to attempt to read at EOF for feof flag to be set so here we
    // use |chunk_size| + 1.
    chunk_size = file_size;
  }

  // We need to attempt to read at EOF for feof flag to be set so here we
// use |chunk_size| + 1.
  chunk_size = std::min<uint64_t>(chunk_size, max_size) + 1;

  cr::File file(path, File::FLAG_READ | File::FLAG_OPEN);
  if (!file.IsValid())
    return false;

  int bytes_read_this_pass;
  size_t bytes_read_so_far = 0;
  bool read_status = true;
  std::string local_contents;
  local_contents.resize(static_cast<size_t>(chunk_size));

  do {
    bytes_read_this_pass = file.ReadAtCurrentPos(
        &local_contents[bytes_read_so_far], 
        static_cast<int>(chunk_size));
    if (bytes_read_this_pass < 0) {
      read_status = false;
      break;
    }

    if ((max_size - bytes_read_so_far) < 
             static_cast<size_t>(bytes_read_this_pass)) {
      // Read more than max_size bytes, bail out.
      bytes_read_so_far = max_size;
      read_status = false;
      break;
    }

    // In case EOF was not reached, iterate again but revert to the default
    // chunk size.
    if (bytes_read_so_far == 0)
      chunk_size = kDefaultChunkSize;

    bytes_read_so_far += bytes_read_this_pass;

    // EOF
    if (bytes_read_this_pass == 0)
      break;

    local_contents.resize(static_cast<size_t>(bytes_read_so_far + chunk_size));
  } while (true);

  if (contents) {
    contents->swap(local_contents);
    contents->resize(bytes_read_so_far);
  }
  return read_status;
}

bool IsDirectoryEmpty(const FilePath& dir_path) {
  FileEnumerator files(dir_path, /*recursive*/false,
      FileEnumerator::FILES | FileEnumerator::DIRECTORIES);
  if (files.Next().empty())
    return true;
  return false;
}

bool CreateDirectory(const FilePath& full_path) {
  return CreateDirectoryAndGetError(full_path, nullptr);
}

bool GetFileSize(const FilePath& file_path, int64_t* file_size) {
  File::Info info;
  if (!GetFileInfo(file_path, &info))
    return false;
  *file_size = info.size;
  return true;
}

bool TouchFile(const FilePath& path,
               const Time& last_accessed,
               const Time& last_modified) {
  int flags = File::FLAG_OPEN | File::FLAG_WRITE_ATTRIBUTES;

#if defined(MINI_CHROMIUM_OS_WIN)
  // On Windows, FILE_FLAG_BACKUP_SEMANTICS is needed to open a directory.
  if (DirectoryExists(path))
    flags |= File::FLAG_BACKUP_SEMANTICS;
#endif

  File file(path, flags);
  if (!file.IsValid())
    return false;

  return file.SetTimes(last_accessed, last_modified);
}

bool CreateTemporaryFileInDir(const FilePath& dir, FilePath* temp_file) {
  return CreateAndOpenTemporaryFileInDir(dir, temp_file).IsValid();
}

FilePath FormatTemporaryFileName(FilePath::StringPieceType identifier) {
  return FilePath(StrCat({identifier, FILE_PATH_LITERAL(".tmp")}));
}

bool CreateNewTempDirectory(const FilePath::StringType& prefix,
                            FilePath* new_temp_path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  FilePath system_temp_dir;
  if (!GetTempDir(&system_temp_dir))
    return false;

  return CreateTemporaryDirInDir(system_temp_dir, prefix, new_temp_path);
}

bool WriteFile(const FilePath& filename, Span<const uint8_t> data) {
  int checkd_size = CheckedNumeric<int>(data.size()).ValueOrDefault(-1);
  if (checkd_size < 0) {
    CR_DLOG(Error) << "|data| is too large.";
    return false;
  }
  return WriteFile(filename, reinterpret_cast<const char*>(data.data()),
                   checkd_size) == checkd_size;
}

bool WriteFile(const FilePath& filename, StringPiece data) {
  return WriteFile(filename, cr::AsConstBytes(MakeSpan(data)));
}

int GetUniquePathNumber(const FilePath& path) {
  CR_DCHECK(!path.empty());
  if (!PathExists(path))
    return 0;

  std::string number;
  for (int count = 1; count <= kMaxUniqueFiles; ++count) {
    StringAppendF(&number, " (%d)", count);
    if (!PathExists(path.InsertBeforeExtensionASCII(number)))
      return count;
    number.clear();
  }

  return -1;
}

FilePath GetUniquePath(const FilePath& path) {
  CR_DCHECK(!path.empty());
  const int uniquifier = GetUniquePathNumber(path);
  if (uniquifier > 0)
    return path.InsertBeforeExtensionASCII(StringPrintf(" (%d)", uniquifier));
  return uniquifier == 0 ? path : cr::FilePath();
}

}  // naemsapce cr
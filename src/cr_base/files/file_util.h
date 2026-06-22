// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

// This file contains utility functions for dealing with the local
// filesystem.

#ifndef MINI_CHROMIUM_SRC_CRBASE_FILES_FILE_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_FILES_FILE_UTIL_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <limits>
#include <set>
#include <string>
#include <vector>

#include "cr_base/compiler_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <sys/stat.h>
#include <unistd.h>
#endif  // defined(MINI_CHROMIUM_OS_WIN)

#include "cr_base/base_export.h"
#include "cr_base/containers/span.h"
#include "cr_base/files/file.h"
#include "cr_base/files/file_path.h"
#include "cr_base/files/scoped_file.h"
#include "cr_base/util/eintr_wrapper.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#undef DeleteFile
#undef ReplaceFile
#undef CopyFile
#undef GetCurrentDirectory
#undef SetCurrentDirectory
#endif  // defined(MINI_CHROMIUM_OS_WIN)

namespace cr {

class Time;

//-----------------------------------------------------------------------------
// Functions that involve filesystem access or modification:

// Returns an absolute version of a relative path. Returns an empty path on
// error. On POSIX, this function fails if the path does not exist. This
// function can result in I/O so it can be slow.
CRBASE_EXPORT FilePath MakeAbsoluteFilePath(const FilePath& input);

// Returns the total number of bytes used by all the files under |root_path|.
// If the path does not exist the function returns 0.
//
// This function is implemented using the FileEnumerator class so it is not
// particularly speedy in any platform.
CRBASE_EXPORT int64_t ComputeDirectorySize(const FilePath& root_path);

// Deletes the given path, whether it's a file or a directory.
// If it's a directory, it's perfectly happy to delete all of the directory's
// contents, but it will not recursively delete subdirectories and their
// contents.
// Returns true if successful, false otherwise. It is considered successful to
// attempt to delete a file that does not exist.
//
// In POSIX environment and if |path| is a symbolic link, this deletes only
// the symlink. (even if the symlink points to a non-existent file)
CRBASE_EXPORT bool DeleteFile(const FilePath& path);

// Deletes the given path, whether it's a file or a directory.
// If it's a directory, it's perfectly happy to delete all of the
// directory's contents, including subdirectories and their contents.
// Returns true if successful, false otherwise. It is considered successful
// to attempt to delete a file that does not exist.
//
// In POSIX environment and if |path| is a symbolic link, this deletes only
// the symlink. (even if the symlink points to a non-existent file)
//
// WARNING: USING THIS EQUIVALENT TO "rm -rf", SO USE WITH CAUTION.
CRBASE_EXPORT bool DeletePathRecursively(const FilePath& path);

#if defined(MINI_CHROMIUM_OS_WIN)
// Schedules to delete the given path, whether it's a file or a directory, until
// the operating system is restarted.
// Note:
// 1) The file/directory to be deleted should exist in a temp folder.
// 2) The directory to be deleted must be empty.
CRBASE_EXPORT bool DeleteFileAfterReboot(const FilePath& path);
#endif  // defined(MINI_CHROMIUM_OS_WIN)

// Moves the given path, whether it's a file or a directory.
// If a simple rename is not possible, such as in the case where the paths are
// on different volumes, this will attempt to copy and delete. Returns
// true for success.
// This function fails if either path contains traversal components ('..').
CRBASE_EXPORT bool Move(const FilePath& from_path, const FilePath& to_path);

// Renames file |from_path| to |to_path|. Both paths must be on the same
// volume, or the function will fail. Destination file will be created
// if it doesn't exist. Prefer this function over Move when dealing with
// temporary files. On Windows it preserves attributes of the target file.
// Returns true on success, leaving *error unchanged.
// Returns false on failure and sets *error appropriately, if it is non-NULL.
CRBASE_EXPORT bool ReplaceFile(const FilePath& from_path,
                               const FilePath& to_path,
                               File::Error* error);

// Copies a single file. Use CopyDirectory() to copy directories.
// This function fails if either path contains traversal components ('..').
// This function also fails if |to_path| is a directory.
//
// On POSIX, if |to_path| is a symlink, CopyFile() will follow the symlink. This
// may have security implications. Use with care.
//
// If |to_path| already exists and is a regular file, it will be overwritten,
// though its permissions will stay the same.
//
// If |to_path| does not exist, it will be created. The new file's permissions
// varies per platform:
//
// - This function keeps the metadata on Windows. The read only bit is not kept.
// - On Mac and iOS, |to_path| retains |from_path|'s permissions, except user
//   read/write permissions are always set.
// - On Linux and Android, |to_path| has user read/write permissions only. i.e.
//   Always 0600.
// - On ChromeOS, |to_path| has user read/write permissions and group/others
//   read permissions. i.e. Always 0644.
CRBASE_EXPORT bool CopyFile(const FilePath& from_path, const FilePath& to_path);

// Copies the given path, and optionally all subdirectories and their contents
// as well.
//
// If there are files existing under to_path, always overwrite. Returns true
// if successful, false otherwise. Wildcards on the names are not supported.
//
// This function has the same metadata behavior as CopyFile().
//
// If you only need to copy a file use CopyFile, it's faster.
CRBASE_EXPORT bool CopyDirectory(const FilePath& from_path,
                                 const FilePath& to_path,
                                 bool recursive);

// Like CopyDirectory() except trying to overwrite an existing file will not
// work and will return false.
CRBASE_EXPORT bool CopyDirectoryExcl(const FilePath& from_path,
                                     const FilePath& to_path,
                                     bool recursive);

// Returns true if the given path exists on the local filesystem,
// false otherwise.
CRBASE_EXPORT bool PathExists(const FilePath& path);

// Returns true if the given path is readable by the user, false otherwise.
CRBASE_EXPORT bool PathIsReadable(const FilePath& path);

// Returns true if the given path is writable by the user, false otherwise.
CRBASE_EXPORT bool PathIsWritable(const FilePath& path);

// Returns true if the given path exists and is a directory, false otherwise.
CRBASE_EXPORT bool DirectoryExists(const FilePath& path);

// Returns the file size. Returns true on success.
CRBASE_EXPORT bool GetFileSize(const FilePath& file_path, int64_t* file_size);

// Returns information about the given file path.
CRBASE_EXPORT bool GetFileInfo(const FilePath& file_path, File::Info* info);

// Sets the time of the last access and the time of the last modification.
CRBASE_EXPORT bool TouchFile(const FilePath& path,
                             const Time& last_accessed,
                             const Time& last_modified);

// Reads the file at |path| into |contents| and returns true on success and
// false on error.  For security reasons, a |path| containing path traversal
// components ('..') is treated as a read error and |contents| is set to empty.
// In case of I/O error, |contents| holds the data that could be read from the
// file before the error occurred.
// |contents| may be NULL, in which case this function is useful for its side
// effect of priming the disk cache (could be used for unit tests).
CRBASE_EXPORT bool ReadFileToString(const FilePath& path, 
                                    std::string* contents);

// Reads the file at |path| into |contents| and returns true on success and
// false on error.  For security reasons, a |path| containing path traversal
// components ('..') is treated as a read error and |contents| is set to empty.
// In case of I/O error, |contents| holds the data that could be read from the
// file before the error occurred.  When the file size exceeds |max_size|, the
// function returns false with |contents| holding the file truncated to
// |max_size|.
// |contents| may be NULL, in which case this function is useful for its side
// effect of priming the disk cache (could be used for unit tests).
CRBASE_EXPORT bool ReadFileToStringWithMaxSize(const FilePath& path,
                                               std::string* contents,
                                               size_t max_size);

// Returns true if the given directory is empty
CRBASE_EXPORT bool IsDirectoryEmpty(const FilePath& dir_path);

// Get the temporary directory provided by the system.
//
// WARNING: In general, you should use CreateTemporaryFile variants below
// instead of this function. Those variants will ensure that the proper
// permissions are set so that other users on the system can't edit them while
// they're open (which can lead to security issues).
CRBASE_EXPORT bool GetTempDir(FilePath* path);

// Get the home directory. This is more complicated than just getenv("HOME")
// as it knows to fall back on getpwent() etc.
//
// You should not generally call this directly. Instead use DIR_HOME with the
// path service which will use this function but cache the value.
// Path service may also override DIR_HOME.
CRBASE_EXPORT FilePath GetHomeDir();

// Creates a directory, as well as creating any parent directories, if they
// don't exist. Returns 'true' on successful creation, or if the directory
// already exists.  The directory is only readable by the current user.
// Returns true on success, leaving *error unchanged.
// Returns false on failure and sets *error appropriately, if it is non-NULL.
CRBASE_EXPORT bool CreateDirectoryAndGetError(const FilePath& full_path,
                                              File::Error* error);

// Backward-compatible convenience method for the above.
CRBASE_EXPORT bool CreateDirectory(const FilePath& full_path);

// Returns a new temporary file in |dir| with a unique name. The file is opened
// for exclusive read, write, and delete access (note: exclusivity is unique to
// Windows). On Windows, the returned file supports File::DeleteOnClose.
// On success, |temp_file| is populated with the full path to the created file.
CRBASE_EXPORT File CreateAndOpenTemporaryFileInDir(const FilePath& dir,
                                                   FilePath* temp_file);

// Same as CreateTemporaryFile but the file is created in |dir|.
CRBASE_EXPORT bool CreateTemporaryFileInDir(const FilePath& dir,
                                            FilePath* temp_file);

// Returns the file name for a temporary file by using a platform-specific
// naming scheme that incorporates |identifier|.
CRBASE_EXPORT FilePath
FormatTemporaryFileName(FilePath::StringPieceType identifier);

// Create a new directory. If prefix is provided, the new directory name is in
// the format of prefixyyyy.
// NOTE: prefix is ignored in the POSIX implementation.
// If success, return true and output the full path of the directory created.
CRBASE_EXPORT bool CreateNewTempDirectory(const FilePath::StringType& prefix,
                                          FilePath* new_temp_path);

// Create a directory within another directory.
// Extra characters will be appended to |prefix| to ensure that the
// new directory does not have the same name as an existing directory.
CRBASE_EXPORT bool CreateTemporaryDirInDir(const FilePath& base_dir,
                                           const FilePath::StringType& prefix,
                                           FilePath* new_dir);

// Sets |real_path| to |path| with symbolic links and junctions expanded.
// On windows, make sure the path starts with a lettered drive.
// |path| must reference a file.  Function will fail if |path| points to
// a directory or to a nonexistent path.  On windows, this function will
// fail if |real_path| would be longer than MAX_PATH characters.
CRBASE_EXPORT bool NormalizeFilePath(const FilePath& path, FilePath* real_path);

#if defined(MINI_CHROMIUM_OS_WIN)
// Given a path in NT native form ("\Device\HarddiskVolumeXX\..."),
// return in |drive_letter_path| the equivalent path that starts with
// a drive letter ("C:\...").  Return false if no such path exists.
CRBASE_EXPORT bool DevicePathToDriveLetterPath(const FilePath& device_path,
                                               FilePath* drive_letter_path);

// Method that wraps the win32 GetLongPathName API, normalizing the specified
// path to its long form. An example where this is needed is when comparing
// temp file paths. If a username isn't a valid 8.3 short file name (even just a
// lengthy name like "user with long name"), Windows will set the TMP and TEMP
// environment variables to be 8.3 paths. ::GetTempPath (called in
// cr::GetTempDir) just uses the value specified by TMP or TEMP, and so can
// return a short path. Returns an empty path on error.
CRBASE_EXPORT FilePath MakeLongFilePath(const FilePath& input);

// Creates a hard link named |to_file| to the file |from_file|. Both paths
// must be on the same volume, and |from_file| may not name a directory.
// Returns true if the hard link is created, false if it fails.
CRBASE_EXPORT bool CreateWinHardLink(const FilePath& to_file,
                                     const FilePath& from_file);
#endif  // defined(MINI_CHROMIUM_OS_WIN)

// Reads at most the given number of bytes from the file into the buffer.
// Returns the number of read bytes, or -1 on error.
CRBASE_EXPORT int ReadFile(const FilePath& filename, char* data, int max_size);

// Writes the given buffer into the file, overwriting any data that was
// previously there.  Returns the number of bytes written, or -1 on error.
// If file doesn't exist, it gets created with read/write permissions for all.
// Note that the other variants of WriteFile() below may be easier to use.
CRBASE_EXPORT int WriteFile(const FilePath& filename, const char* data,
                            int size);

// Writes |data| into the file, overwriting any data that was previously there.
// Returns true if and only if all of |data| was written. If the file does not
// exist, it gets created with read/write permissions for all.
CRBASE_EXPORT bool WriteFile(const FilePath& filename, 
                             Span<const uint8_t> data);

// Another WriteFile() variant that takes a StringPiece so callers don't have to
// do manual conversions from a char span to a uint8_t span.
CRBASE_EXPORT bool WriteFile(const FilePath& filename, StringPiece data);

// Appends |data| to |filename|.  Returns true iff |size| bytes of |data| were
// written to |filename|.
CRBASE_EXPORT bool AppendToFile(const FilePath& filename,
                                const char* data,
                                int size);

// Gets the current working directory for the process.
CRBASE_EXPORT bool GetCurrentDirectory(FilePath* path);

// Sets the current working directory for the process.
CRBASE_EXPORT bool SetCurrentDirectory(const FilePath& path);

// The largest value attempted by GetUniquePath{Number,}.
enum { kMaxUniqueFiles = 100 };

// Returns the number N that makes |path| unique when formatted as " (N)" in a
// suffix to its basename before any file extension, where N is a number between
// 1 and 100 (inclusive). Returns 0 if |path| does not exist (meaning that it is
// unique as-is), or -1 if no such number can be found.
CRBASE_EXPORT int GetUniquePathNumber(const FilePath& path);

// Returns |path| if it does not exist. Otherwise, returns |path| with the
// suffix " (N)" appended to its basename before any file extension, where N is
// a number between 1 and 100 (inclusive). Returns an empty path if no such
// number can be found.
CRBASE_EXPORT FilePath GetUniquePath(const FilePath& path);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_FILES_FILE_UTIL_H_
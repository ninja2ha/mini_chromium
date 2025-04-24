// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_POSIX)
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "crbase/base_export.h"
#include "crbase/files/file.h"
#include "crbase/files/file_path.h"
#include "crbase/strings/string16.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include "crbase/file_descriptor_posix.h"
#include "crbase/logging.h"
#include "crbase/posix/eintr_wrapper.h"
#endif

namespace cr {

class Environment;
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
// If it's a directory, it's perfectly happy to delete all of the
// directory's contents.  Passing true to recursive deletes
// subdirectories and their contents as well.
// Returns true if successful, false otherwise. It is considered successful
// to attempt to delete a file that does not exist.
//
// In POSIX environment and if |path| is a symbolic link, this deletes only
// the symlink. (even if the symlink points to a non-existent file)
//
// WARNING: USING THIS WITH recursive==true IS EQUIVALENT
//          TO "rm -rf", SO USE WITH CAUTION.
//
// Note: The |recursive| parameter is in the process of being removed. Use
// DeleteFileRecursively() instead. See https://crbug.com/1009837
CRBASE_EXPORT bool DeleteFile(const FilePath& path, bool recursive);

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
CRBASE_EXPORT bool DeleteFileRecursively(const FilePath& path);

#if defined(MINI_CHROMIUM_OS_WIN)
// Schedules to delete the given path, whether it's a file or a directory, until
// the operating system is restarted.
// Note:
// 1) The file/directory to be deleted should exist in a temp folder.
// 2) The directory to be deleted must be empty.
CRBASE_EXPORT bool DeleteFileAfterReboot(const FilePath& path);
#endif

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

// Returns true if the given path is writable by the user, false otherwise.
CRBASE_EXPORT bool PathIsWritable(const FilePath& path);

// Returns true if the given path exists and is a directory, false otherwise.
CRBASE_EXPORT bool DirectoryExists(const FilePath& path);

// Returns true if the contents of the two files given are equal, false
// otherwise.  If either file can't be read, returns false.
CRBASE_EXPORT bool ContentsEqual(const FilePath& filename1,
                                 const FilePath& filename2);

// Returns true if the contents of the two text files given are equal, false
// otherwise.  This routine treats "\r\n" and "\n" as equivalent.
CRBASE_EXPORT bool TextContentsEqual(const FilePath& filename1,
                                     const FilePath& filename2);

// Reads the file at |path| into |contents| and returns true on success and
// false on error.  For security reasons, a |path| containing path traversal
// components ('..') is treated as a read error and |contents| is set to empty.
// In case of I/O error, |contents| holds the data that could be read from the
// file before the error occurred.
// |contents| may be NULL, in which case this function is useful for its side
// effect of priming the disk cache (could be used for unit tests).
CRBASE_EXPORT bool ReadFileToString(const FilePath& path, std::string* contents);

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

#if defined(MINI_CHROMIUM_OS_POSIX)

// Read exactly |bytes| bytes from file descriptor |fd|, storing the result
// in |buffer|. This function is protected against EINTR and partial reads.
// Returns true iff |bytes| bytes have been successfully read from |fd|.
CRBASE_EXPORT bool ReadFromFD(int fd, char* buffer, size_t bytes);

// Performs the same function as CreateAndOpenTemporaryFileInDir(), but returns
// the file-descriptor wrapped in a ScopedFD, rather than wrapped in a FILE.
CRBASE_EXPORT ScopedFD CreateAndOpenFdForTemporaryFileInDir(const FilePath& dir,
                                                            FilePath* path);

#endif  // defined(OS_POSIX) || defined(OS_FUCHSIA)

#if defined(MINI_CHROMIUM_OS_POSIX)

// Creates a symbolic link at |symlink| pointing to |target|.  Returns
// false on failure.
CRBASE_EXPORT bool CreateSymbolicLink(const FilePath& target,
                                      const FilePath& symlink);

// Reads the given |symlink| and returns where it points to in |target|.
// Returns false upon failure.
CRBASE_EXPORT bool ReadSymbolicLink(const FilePath& symlink, FilePath* target);

// Bits and masks of the file permission.
enum FilePermissionBits {
  FILE_PERMISSION_MASK              = S_IRWXU | S_IRWXG | S_IRWXO,
  FILE_PERMISSION_USER_MASK         = S_IRWXU,
  FILE_PERMISSION_GROUP_MASK        = S_IRWXG,
  FILE_PERMISSION_OTHERS_MASK       = S_IRWXO,

  FILE_PERMISSION_READ_BY_USER      = S_IRUSR,
  FILE_PERMISSION_WRITE_BY_USER     = S_IWUSR,
  FILE_PERMISSION_EXECUTE_BY_USER   = S_IXUSR,
  FILE_PERMISSION_READ_BY_GROUP     = S_IRGRP,
  FILE_PERMISSION_WRITE_BY_GROUP    = S_IWGRP,
  FILE_PERMISSION_EXECUTE_BY_GROUP  = S_IXGRP,
  FILE_PERMISSION_READ_BY_OTHERS    = S_IROTH,
  FILE_PERMISSION_WRITE_BY_OTHERS   = S_IWOTH,
  FILE_PERMISSION_EXECUTE_BY_OTHERS = S_IXOTH,
};

// Reads the permission of the given |path|, storing the file permission
// bits in |mode|. If |path| is symbolic link, |mode| is the permission of
// a file which the symlink points to.
CRBASE_EXPORT bool GetPosixFilePermissions(const FilePath& path, int* mode);
// Sets the permission of the given |path|. If |path| is symbolic link, sets
// the permission of a file which the symlink points to.
CRBASE_EXPORT bool SetPosixFilePermissions(const FilePath& path, int mode);

// Returns true iff |executable| can be found in any directory specified by the
// environment variable in |env|.
CRBASE_EXPORT bool ExecutableExistsInPath(Environment* env,
                                          const FilePath::StringType& executable);

#if defined(MINI_CHROMIUM_OS_LINUX)
// Determine if files under a given |path| can be mapped and then mprotect'd
// PROT_EXEC. This depends on the mount options used for |path|, which vary
// among different Linux distributions and possibly local configuration. It also
// depends on details of kernel--ChromeOS uses the noexec option for /dev/shm
// but its kernel allows mprotect with PROT_EXEC anyway.
CRBASE_EXPORT bool IsPathExecutable(const FilePath& path);
#endif  // OS_LINUX || OS_AIX

#endif  // MINI_CHROMIUM_OS_POSIX

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

// Creates a temporary file. The full path is placed in |path|, and the
// function returns true if was successful in creating the file. The file will
// be empty and all handles closed after this function returns.
CRBASE_EXPORT bool CreateTemporaryFile(FilePath* path);

// Same as CreateTemporaryFile but the file is created in |dir|.
CRBASE_EXPORT bool CreateTemporaryFileInDir(const FilePath& dir,
                                            FilePath* temp_file);

// Create and open a temporary file.  File is opened for read/write.
// The full path is placed in |path|.
// Returns a handle to the opened file or NULL if an error occurred.
CRBASE_EXPORT FILE* CreateAndOpenTemporaryFile(FilePath* path);

// Similar to CreateAndOpenTemporaryFile, but the file is created in |dir|.
CRBASE_EXPORT FILE* CreateAndOpenTemporaryFileInDir(const FilePath& dir,
                                                    FilePath* path);

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

// Creates a directory, as well as creating any parent directories, if they
// don't exist. Returns 'true' on successful creation, or if the directory
// already exists.  The directory is only readable by the current user.
// Returns true on success, leaving *error unchanged.
// Returns false on failure and sets *error appropriately, if it is non-NULL.
CRBASE_EXPORT bool CreateDirectoryAndGetError(const FilePath& full_path,
                                              File::Error* error);

// Backward-compatible convenience method for the above.
CRBASE_EXPORT bool CreateDirectory(const FilePath& full_path);

// Returns the file size. Returns true on success.
CRBASE_EXPORT bool GetFileSize(const FilePath& file_path, int64_t* file_size);

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
// base::GetTempDir) just uses the value specified by TMP or TEMP, and so can
// return a short path. Returns an empty path on error.
CRBASE_EXPORT FilePath MakeLongFilePath(const FilePath& input);

// Creates a hard link named |to_file| to the file |from_file|. Both paths
// must be on the same volume, and |from_file| may not name a directory.
// Returns true if the hard link is created, false if it fails.
CRBASE_EXPORT bool CreateWinHardLink(const FilePath& to_file,
                                   const FilePath& from_file);
#endif

// This function will return if the given file is a symlink or not.
CRBASE_EXPORT bool IsLink(const FilePath& file_path);

// Returns information about the given file path.
CRBASE_EXPORT bool GetFileInfo(const FilePath& file_path, File::Info* info);

// Sets the time of the last access and the time of the last modification.
CRBASE_EXPORT bool TouchFile(const FilePath& path,
                             const Time& last_accessed,
                             const Time& last_modified);

// Wrapper for fopen-like calls. Returns non-NULL FILE* on success. The
// underlying file descriptor (POSIX) or handle (Windows) is unconditionally
// configured to not be propagated to child processes.
CRBASE_EXPORT FILE* OpenFile(const FilePath& filename, const char* mode);

// Closes file opened by OpenFile. Returns true on success.
CRBASE_EXPORT bool CloseFile(FILE* file);

// Associates a standard FILE stream with an existing File. Note that this
// functions take ownership of the existing File.
CRBASE_EXPORT FILE* FileToFILE(File file, const char* mode);

// Truncates an open file to end at the location of the current file pointer.
// This is a cross-platform analog to Windows' SetEndOfFile() function.
CRBASE_EXPORT bool TruncateFile(FILE* file);

// Reads at most the given number of bytes from the file into the buffer.
// Returns the number of read bytes, or -1 on error.
CRBASE_EXPORT int ReadFile(const FilePath& filename, char* data, int max_size);

// Writes the given buffer into the file, overwriting any data that was
// previously there.  Returns the number of bytes written, or -1 on error.
// If file doesn't exist, it gets created with read/write permissions for all.
CRBASE_EXPORT int WriteFile(const FilePath& filename, const char* data,
                            int size);

#if defined(MINI_CHROMIUM_OS_POSIX)
// Appends |data| to |fd|. Does not close |fd| when done.  Returns true iff
// |size| bytes of |data| were written to |fd|.
CRBASE_EXPORT bool WriteFileDescriptor(const int fd, const char* data, 
                                       int size);

// Allocates disk space for the file referred to by |fd| for the byte range
// starting at |offset| and continuing for |size| bytes. The file size will be
// changed if |offset|+|len| is greater than the file size. Zeros will fill the
// new space.
// After a successful call, subsequent writes into the specified range are
// guaranteed not to fail because of lack of disk space.
CRBASE_EXPORT bool AllocateFileRegion(File* file, int64_t offset, size_t size);
#endif

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

#if defined(MINI_CHROMIUM_OS_POSIX)

// Creates a pipe. Returns true on success, otherwise false.
// On success, |read_fd| will be set to the fd of the read side, and
// |write_fd| will be set to the one of write side. If |non_blocking|
// is set the pipe will be created with O_NONBLOCK|O_CLOEXEC flags set
// otherwise flag is set to zero (default).
CRBASE_EXPORT bool CreatePipe(ScopedFD* read_fd,
                              ScopedFD* write_fd,
                              bool non_blocking = false);

// Creates a non-blocking, close-on-exec pipe.
// This creates a non-blocking pipe that is not intended to be shared with any
// child process. This will be done atomically if the operating system supports
// it. Returns true if it was able to create the pipe, otherwise false.
CRBASE_EXPORT bool CreateLocalNonBlockingPipe(int fds[2]);

// Sets the given |fd| to close-on-exec mode.
// Returns true if it was able to set it in the close-on-exec mode, otherwise
// false.
CRBASE_EXPORT bool SetCloseOnExec(int fd);

// Test that |path| can only be changed by a given user and members of
// a given set of groups.
// Specifically, test that all parts of |path| under (and including) |base|:
// * Exist.
// * Are owned by a specific user.
// * Are not writable by all users.
// * Are owned by a member of a given set of groups, or are not writable by
//   their group.
// * Are not symbolic links.
// This is useful for checking that a config file is administrator-controlled.
// |base| must contain |path|.
CRBASE_EXPORT bool VerifyPathControlledByUser(const FilePath& base,
                                              const FilePath& path,
                                              uid_t owner_uid,
                                              const std::set<gid_t>& group_gids);
#endif  // defined(OS_POSIX) || defined(OS_FUCHSIA)

// Returns the maximum length of path component on the volume containing
// the directory |path|, in the number of FilePath::CharType, or -1 on failure.
CRBASE_EXPORT int GetMaximumPathComponentLength(const FilePath& path);

#if defined(MINI_CHROMIUM_OS_LINUX)
// Broad categories of file systems as returned by statfs() on Linux.
enum FileSystemType {
  FILE_SYSTEM_UNKNOWN,  // statfs failed.
  FILE_SYSTEM_0,        // statfs.f_type == 0 means unknown, may indicate AFS.
  FILE_SYSTEM_ORDINARY,       // on-disk filesystem like ext2
  FILE_SYSTEM_NFS,
  FILE_SYSTEM_SMB,
  FILE_SYSTEM_CODA,
  FILE_SYSTEM_MEMORY,         // in-memory file system
  FILE_SYSTEM_CGROUP,         // cgroup control.
  FILE_SYSTEM_OTHER,          // any other value.
  FILE_SYSTEM_TYPE_COUNT
};

// Attempts determine the FileSystemType for |path|.
// Returns false if |path| doesn't exist.
CRBASE_EXPORT bool GetFileSystemType(const FilePath& path, FileSystemType* type);
#endif

#if defined(MINI_CHROMIUM_OS_POSIX)
// Get a temporary directory for shared memory files. The directory may depend
// on whether the destination is intended for executable files, which in turn
// depends on how /dev/shmem was mounted. As a result, you must supply whether
// you intend to create executable shmem segments so this function can find
// an appropriate location.
CRBASE_EXPORT bool GetShmemTempDir(bool executable, FilePath* path);
#endif

// Internal --------------------------------------------------------------------

namespace internal {

// Same as Move but allows paths with traversal components.
// Use only with extreme care.
CRBASE_EXPORT bool MoveUnsafe(const FilePath& from_path,
                              const FilePath& to_path);

#if defined(MINI_CHROMIUM_OS_WIN)
// Copy from_path to to_path recursively and then delete from_path recursively.
// Returns true if all operations succeed.
// This function simulates Move(), but unlike Move() it works across volumes.
// This function is not transactional.
CRBASE_EXPORT bool CopyAndDeleteDirectory(const FilePath& from_path,
                                          const FilePath& to_path);
#endif  // defined(MINI_CHROMIUM_OS_WIN)

// Used by PreReadFile() when no kernel support for prefetching is available.
///bool PreReadFileSlow(const FilePath& file_path, int64_t max_bytes);

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_FILES_FILE_UTIL_H_
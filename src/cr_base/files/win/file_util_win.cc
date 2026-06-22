// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "cr_base/files/file_util.h"

#include "cr_base/compiler_specific.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
typedef struct IUnknown IUnknown;
#include <windows.h>

CR_MSVC_PUSH_DISABLE_WARNING(4091);
#include <shellapi.h>
#include <shlobj.h>
CR_MSVC_POP_WARNING(pop);

#include "cr_base/logging/logging.h"
#include "cr_base/debug/alias.h"
#include "cr_base/guid.h"
#include "cr_base/rand_util.h"
#include "cr_base/strings/string_util.h"
#include "cr_base/strings/string_number_conversions.h"
#include "cr_base/strings/utf_string_conversions.h"
#include "cr_base/files/file_enumerator.h"
#include "cr_base/win/scoped_handle.h"

#undef DeleteFile
#undef ReplaceFile
#undef CopyFile
#undef GetCurrentDirectory
#undef SetCurrentDirectory

namespace cr {

namespace {

const DWORD kFileShareAll =
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

// Returns the Win32 last error code or ERROR_SUCCESS if the last error code is
// ERROR_FILE_NOT_FOUND or ERROR_PATH_NOT_FOUND. This is useful in cases where
// the absence of a file or path is a success condition (e.g., when attempting
// to delete an item in the filesystem).
DWORD ReturnLastErrorOrSuccessOnNotFound() {
  const DWORD error_code = ::GetLastError();
  return (error_code == ERROR_FILE_NOT_FOUND ||
          error_code == ERROR_PATH_NOT_FOUND)
             ? ERROR_SUCCESS
             : error_code;
}

// Deletes all files and directories in a path.
// Returns ERROR_SUCCESS on success or the Windows error code corresponding to
// the first error encountered. ERROR_FILE_NOT_FOUND and ERROR_PATH_NOT_FOUND
// are considered success conditions, and are therefore never returned.
DWORD DoDeleteFileRecursive(const FilePath& path,
                            const FilePath::StringType& pattern,
                            bool recursive) {
  FileEnumerator traversal(path, false,
                           FileEnumerator::FILES | FileEnumerator::DIRECTORIES,
                           pattern);
  DWORD result = ERROR_SUCCESS;
  for (FilePath current = traversal.Next(); !current.empty();
       current = traversal.Next()) {
    // Try to clear the read-only bit if we find it.
    FileEnumerator::FileInfo info = traversal.GetInfo();
    if ((info.find_data().dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        (recursive || !info.IsDirectory())) {
      ::SetFileAttributesW(
          current.value().c_str(),
          info.find_data().dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
    }

    DWORD this_result = ERROR_SUCCESS;
    if (info.IsDirectory()) {
      if (recursive) {
        this_result = DoDeleteFileRecursive(current, pattern, true);
        CR_DCHECK(static_cast<LONG>(this_result) != ERROR_FILE_NOT_FOUND);
        CR_DCHECK(static_cast<LONG>(this_result) != ERROR_PATH_NOT_FOUND);
        if (this_result == ERROR_SUCCESS &&
            !::RemoveDirectoryW(current.value().c_str())) {
          this_result = ReturnLastErrorOrSuccessOnNotFound();
        }
      }
    } else if (!::DeleteFileW(current.value().c_str())) {
      this_result = ReturnLastErrorOrSuccessOnNotFound();
    }
    if (result == ERROR_SUCCESS)
      result = this_result;
  }
  return result;
}

bool DoCopyFile(const FilePath& from_path,
                const FilePath& to_path,
                bool fail_if_exists) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  if (from_path.ReferencesParent() || to_path.ReferencesParent())
    return false;

  // NOTE: I suspect we could support longer paths, but that would involve
  // analyzing all our usage of files.
  if (from_path.value().length() >= MAX_PATH ||
      to_path.value().length() >= MAX_PATH) {
    return false;
  }

  // Unlike the posix implementation that copies the file manually and discards
  // the ACL bits, CopyFile() copies the complete SECURITY_DESCRIPTOR and access
  // bits, which is usually not what we want. We can't do much about the
  // SECURITY_DESCRIPTOR but at least remove the read only bit.
  const wchar_t* dest = to_path.value().c_str();
  if (!::CopyFileW(from_path.value().c_str(), dest, fail_if_exists)) {
    // Copy failed.
    return false;
  }
  DWORD attrs = ::GetFileAttributesW(dest);
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  if (attrs & FILE_ATTRIBUTE_READONLY) {
    ::SetFileAttributesW(dest, attrs & ~FILE_ATTRIBUTE_READONLY);
  }
  return true;
}

bool DoCopyDirectory(const FilePath& from_path,
                     const FilePath& to_path,
                     bool recursive,
                     bool fail_if_exists) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  // NOTE: I suspect we could support longer paths, but that would involve
  // analyzing all our usage of files.
  if (from_path.value().length() >= MAX_PATH ||
      to_path.value().length() >= MAX_PATH) {
    return false;
  }

  // This function does not properly handle destinations within the source.
  FilePath real_to_path = to_path;
  if (PathExists(real_to_path)) {
    real_to_path = MakeAbsoluteFilePath(real_to_path);
    if (real_to_path.empty())
      return false;
  } else {
    real_to_path = MakeAbsoluteFilePath(real_to_path.DirName());
    if (real_to_path.empty())
      return false;
  }
  FilePath real_from_path = MakeAbsoluteFilePath(from_path);
  if (real_from_path.empty())
    return false;
  if (real_to_path == real_from_path || real_from_path.IsParent(real_to_path))
    return false;

  int traverse_type = FileEnumerator::FILES;
  if (recursive)
    traverse_type |= FileEnumerator::DIRECTORIES;
  FileEnumerator traversal(from_path, recursive, traverse_type);

  if (!PathExists(from_path)) {
    CR_DLOG(Error) << "CopyDirectory() couldn't stat source directory: "
                   << from_path.value().c_str();
    return false;
  }
  // TODO(maruel): This is not necessary anymore.
  CR_DCHECK(recursive || DirectoryExists(from_path));

  FilePath current = from_path;
  bool from_is_dir = DirectoryExists(from_path);
  bool success = true;
  FilePath from_path_base = from_path;
  if (recursive && DirectoryExists(to_path)) {
    // If the destination already exists and is a directory, then the
    // top level of source needs to be copied.
    from_path_base = from_path.DirName();
  }

  while (success && !current.empty()) {
    // current is the source path, including from_path, so append
    // the suffix after from_path to to_path to create the target_path.
    FilePath target_path(to_path);
    if (from_path_base != current) {
      if (!from_path_base.AppendRelativePath(current, &target_path)) {
        success = false;
        break;
      }
    }

    if (from_is_dir) {
      if (!DirectoryExists(target_path) &&
          !::CreateDirectoryW(target_path.value().c_str(), NULL)) {
        CR_DLOG(Error) << "CopyDirectory() couldn't create directory: "
                       << target_path.value().c_str();
        success = false;
      }
    } else if (!DoCopyFile(current, target_path, fail_if_exists)) {
      CR_DLOG(Error) << "CopyDirectory() couldn't create file: "
                     << target_path.value().c_str();
      success = false;
    }

    current = traversal.Next();
    if (!current.empty())
      from_is_dir = traversal.GetInfo().IsDirectory();
  }

  return success;
}

// Returns ERROR_SUCCESS on success, or a Windows error code on failure.
DWORD DoDeleteFile(const FilePath& path, bool recursive) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  if (path.empty())
    return ERROR_SUCCESS;

  if (path.value().length() >= MAX_PATH)
    return ERROR_BAD_PATHNAME;

  // Handle any path with wildcards.
  if (path.BaseName().value().find_first_of(FILE_PATH_LITERAL("*?")) !=
      FilePath::StringType::npos) {
    const DWORD error_code = DoDeleteFileRecursive(path.DirName(), 
                                                   path.BaseName().value(),
                                                   recursive);
    CR_DCHECK(static_cast<LONG>(error_code) != ERROR_FILE_NOT_FOUND);
    CR_DCHECK(static_cast<LONG>(error_code) != ERROR_PATH_NOT_FOUND);
    return error_code;
  }

  // Report success if the file or path does not exist.
  const DWORD attr = ::GetFileAttributesW(path.value().c_str());
  if (attr == INVALID_FILE_ATTRIBUTES)
    return ReturnLastErrorOrSuccessOnNotFound();

  // Clear the read-only bit if it is set.
  if ((attr & FILE_ATTRIBUTE_READONLY) &&
      !::SetFileAttributesW(path.value().c_str(),
                            attr & ~FILE_ATTRIBUTE_READONLY)) {
    // It's possible for |path| to be gone now under a race with other deleters.
    return ReturnLastErrorOrSuccessOnNotFound();
  }

  // Perform a simple delete on anything that isn't a directory.
  if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
    return ::DeleteFileW(path.value().c_str())
               ? ERROR_SUCCESS
               : ReturnLastErrorOrSuccessOnNotFound();
  }

  if (recursive) {
    const DWORD error_code =
        DoDeleteFileRecursive(path, FILE_PATH_LITERAL("*"), true);
    CR_DCHECK(static_cast<LONG>(error_code) != ERROR_FILE_NOT_FOUND);
    CR_DCHECK(static_cast<LONG>(error_code) != ERROR_PATH_NOT_FOUND);
    if (error_code != ERROR_SUCCESS)
      return error_code;
  }
  return ::RemoveDirectoryW(path.value().c_str())
             ? ERROR_SUCCESS
             : ReturnLastErrorOrSuccessOnNotFound();
}

bool DoPathHasAccess(const FilePath& path,
                     DWORD dir_desired_access,
                     DWORD file_desired_access) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  const wchar_t* const path_str = path.value().c_str();
  DWORD fileattr = ::GetFileAttributesW(path_str);
  if (fileattr == INVALID_FILE_ATTRIBUTES)
    return false;

  bool is_directory = fileattr & FILE_ATTRIBUTE_DIRECTORY;
  DWORD desired_access =
      is_directory ? dir_desired_access : file_desired_access;
  DWORD flags_and_attrs =
      is_directory ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL;

  win::ScopedHandle file(::CreateFileW(path_str, desired_access, kFileShareAll,
                                       nullptr, OPEN_EXISTING, flags_and_attrs,
                                       nullptr));

  return file.IsValid();
}

}  // namespace

FilePath MakeAbsoluteFilePath(const FilePath& input) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  wchar_t file_path[MAX_PATH];
  if (!_wfullpath(file_path, input.value().c_str(), MAX_PATH))
    return FilePath();
  return FilePath(file_path);
}

bool DeleteFile(const FilePath& path) {
  return DoDeleteFile(path, /*recursive=*/false);
}

bool DeletePathRecursively(const FilePath& path) {
  return DoDeleteFile(path, /*recursive=*/true);
}

bool DeleteFileAfterReboot(const FilePath& path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  if (path.value().length() >= MAX_PATH)
    return false;

  return ::MoveFileExW(path.value().c_str(), nullptr,
                       MOVEFILE_DELAY_UNTIL_REBOOT);
}

bool Move(const FilePath& from_path, const FilePath& to_path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  // NOTE: I suspect we could support longer paths, but that would involve
  // analyzing all our usage of files.
  if (from_path.value().length() >= MAX_PATH ||
      to_path.value().length() >= MAX_PATH) {
    return false;
  }
  if (::MoveFileExW(from_path.value().c_str(), to_path.value().c_str(),
                    MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != 0)
    return true;

  // Keep the last error value from MoveFileEx around in case the below
  // fails.
  bool ret = false;
  DWORD last_error = ::GetLastError();

  if (DirectoryExists(from_path)) {
    // MoveFileEx fails if moving directory across volumes. We will simulate
    // the move by using Copy and Delete. Ideally we could check whether
    // from_path and to_path are indeed in different volumes.
    ret = (CopyDirectory(from_path, to_path, true) && 
           DeletePathRecursively(from_path));
  }

  if (!ret) {
    // Leave a clue about what went wrong so that it can be (at least) picked
    // up by a PLOG entry.
    ::SetLastError(last_error);
  }

  return ret;
}

bool ReplaceFile(const FilePath& from_path,
                 const FilePath& to_path,
                 File::Error* error) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  // Alias paths for investigation of shutdown hangs. crbug.com/1054164
  FilePath::CharType from_path_str[MAX_PATH];
  cr::wcslcpy(from_path_str, from_path.value().c_str(),
              cr::size(from_path_str));
  cr::debug::Alias(from_path_str);
  FilePath::CharType to_path_str[MAX_PATH];
  cr::wcslcpy(to_path_str, to_path.value().c_str(), cr::size(to_path_str));
  cr::debug::Alias(to_path_str);

  // Assume that |to_path| already exists and try the normal replace. This will
  // fail with ERROR_FILE_NOT_FOUND if |to_path| does not exist. When writing to
  // a network share, we may not be able to change the ACLs. Ignore ACL errors
  // then (REPLACEFILE_IGNORE_MERGE_ERRORS).
  if (::ReplaceFileW(to_path.value().c_str(), from_path.value().c_str(), NULL,
                     REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL)) {
    return true;
  }

  File::Error replace_error = File::OSErrorToFileError(GetLastError());

  // Try a simple move next. It will only succeed when |to_path| doesn't already
  // exist.
  if (::MoveFileW(from_path.value().c_str(), to_path.value().c_str()))
    return true;

  // In the case of FILE_ERROR_NOT_FOUND from ReplaceFile, it is likely that
  // |to_path| does not exist. In this case, the more relevant error comes
  // from the call to MoveFile.
  if (error) {
    *error = replace_error == File::FILE_ERROR_NOT_FOUND
                 ? File::GetLastFileError()
                 : replace_error;
  }
  return false;
}

bool CopyFile(const FilePath& from_path, const FilePath& to_path) {
  return DoCopyFile(from_path, to_path, false);
}

bool CopyDirectory(const FilePath& from_path,
                   const FilePath& to_path,
                   bool recursive) {
  return DoCopyDirectory(from_path, to_path, recursive, false);
}

bool CopyDirectoryExcl(const FilePath& from_path,
                       const FilePath& to_path,
                       bool recursive) {
  return DoCopyDirectory(from_path, to_path, recursive, true);
}

bool PathExists(const FilePath& path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return 
      (::GetFileAttributesW(path.value().c_str()) != INVALID_FILE_ATTRIBUTES);
}

bool PathIsReadable(const FilePath& path) {
  return DoPathHasAccess(path, FILE_LIST_DIRECTORY, GENERIC_READ);
}

bool PathIsWritable(const FilePath& path) {
  return DoPathHasAccess(path, FILE_ADD_FILE, GENERIC_WRITE);
}

bool DirectoryExists(const FilePath& path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DWORD fileattr = ::GetFileAttributesW(path.value().c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

bool GetTempDir(FilePath* path) {
  wchar_t temp_path[MAX_PATH + 1];
  DWORD path_len = ::GetTempPathW(MAX_PATH, temp_path);
  if (path_len >= MAX_PATH || path_len <= 0)
    return false;
  // TODO(evanm): the old behavior of this function was to always strip the
  // trailing slash.  We duplicate this here, but it shouldn't be necessary
  // when everyone is using the appropriate FilePath APIs.
  *path = FilePath(temp_path).StripTrailingSeparators();
  return true;
}

FilePath GetHomeDir() {
  wchar_t result[MAX_PATH];
  if (SUCCEEDED(::SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 
                                   SHGFP_TYPE_CURRENT, result)) &&
      result[0]) {
    return FilePath(result);
  }

  // Fall back to the temporary directory on failure.
  FilePath temp;
  if (GetTempDir(&temp))
    return temp;

  // Last resort.
  return FilePath(FILE_PATH_LITERAL("C:\\"));
}

bool CreateDirectoryAndGetError(const FilePath& full_path,
                                File::Error* error) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  // If the path exists, we've succeeded if it's a directory, failed otherwise.
  const wchar_t* const full_path_str = full_path.value().c_str();
  const DWORD fileattr = ::GetFileAttributesW(full_path_str);
  if (fileattr != INVALID_FILE_ATTRIBUTES) {
    if ((fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      return true;
    }
    CR_DLOG(Warning) << "CreateDirectory(" << full_path_str << "), "
                     << "conflicts with existing file.";
    if (error)
      *error = File::FILE_ERROR_NOT_A_DIRECTORY;
    ::SetLastError(ERROR_FILE_EXISTS);
    return false;
  }

  // Invariant:  Path does not exist as file or directory.

  // Attempt to create the parent recursively.  This will immediately return
  // true if it already exists, otherwise will create all required parent
  // directories starting with the highest-level missing parent.
  FilePath parent_path(full_path.DirName());
  if (parent_path.value() == full_path.value()) {
    if (error)
      *error = File::FILE_ERROR_NOT_FOUND;
    ::SetLastError(ERROR_FILE_NOT_FOUND);
    return false;
  }
  if (!CreateDirectoryAndGetError(parent_path, error)) {
    CR_DLOG(Warning) << "Failed to create one of the parent directories.";
    CR_DCHECK(!error || *error != File::FILE_OK);
    return false;
  }

  if (::CreateDirectoryW(full_path_str, NULL))
    return true;

  const DWORD error_code = ::GetLastError();
  if (error_code == ERROR_ALREADY_EXISTS && DirectoryExists(full_path)) {
    // This error code ERROR_ALREADY_EXISTS doesn't indicate whether we were
    // racing with someone creating the same directory, or a file with the same
    // path.  If DirectoryExists() returns true, we lost the race to create the
    // same directory.
    return true;
  }
  if (error)
    *error = File::OSErrorToFileError(error_code);
  ::SetLastError(error_code);
  CR_DPLOG(Warning) << "Failed to create directory " << full_path_str;
  return false;
}

File CreateAndOpenTemporaryFileInDir(const FilePath& dir, FilePath* temp_file) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  // Open the file with exclusive r/w/d access, and allow the caller to decide
  // to mark it for deletion upon close after the fact.
  constexpr uint32_t kFlags = File::FLAG_CREATE | File::FLAG_READ |
                              File::FLAG_WRITE | File::FLAG_EXCLUSIVE_READ |
                              File::FLAG_EXCLUSIVE_WRITE |
                              File::FLAG_CAN_DELETE_ON_CLOSE;

  // Use GUID instead of ::GetTempFileName() to generate unique file names.
  // "Due to the algorithm used to generate file names, GetTempFileName can
  // perform poorly when creating a large number of files with the same prefix.
  // In such cases, it is recommended that you construct unique file names based
  // on GUIDs."
  // https://msdn.microsoft.com/library/windows/desktop/aa364991.aspx

  FilePath temp_name;
  File file;

  // Although it is nearly impossible to get a duplicate name with GUID, we
  // still use a loop here in case it happens.
  for (int i = 0; i < 100; ++i) {
    temp_name = dir.Append(FormatTemporaryFileName(UTF8ToWide(GenerateGUID())));
    file.Initialize(temp_name, kFlags);
    if (file.IsValid())
      break;
  }

  if (!file.IsValid()) {
    CR_DPLOG(Warning) << "Failed to get temporary file name in " << dir.value();
    return file;
  }

  wchar_t long_temp_name[MAX_PATH + 1];
  const DWORD long_name_len =
      ::GetLongPathNameW(temp_name.value().c_str(), long_temp_name, MAX_PATH);
  if (long_name_len != 0 && long_name_len <= MAX_PATH) {
    *temp_file =
        FilePath(FilePath::StringPieceType(long_temp_name, long_name_len));
  } else {
    // GetLongPathName() failed, but we still have a temporary file.
    *temp_file = std::move(temp_name);
  }

  return file;
}

bool CreateTemporaryDirInDir(const FilePath& base_dir,
                             const FilePath::StringType& prefix,
                             FilePath* new_dir) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  FilePath path_to_create;

  for (int count = 0; count < 50; ++count) {
    // Try create a new temporary directory with random generated name. If
    // the one exists, keep trying another path name until we reach some limit.
    std::wstring new_dir_name;
    new_dir_name.assign(prefix);
    new_dir_name.append(AsWString(NumberToString16(::GetCurrentProcessId())));
    new_dir_name.push_back('_');
    new_dir_name.append(AsWString(
        NumberToString16(RandInt(0, std::numeric_limits<int32_t>::max()))));

    path_to_create = base_dir.Append(new_dir_name);
    if (::CreateDirectoryW(path_to_create.value().c_str(), NULL)) {
      *new_dir = path_to_create;
      return true;
    }
  }

  return false;
}

bool NormalizeFilePath(const FilePath& path, FilePath* real_path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  File file(path, File::FLAG_OPEN | File::FLAG_READ | File::FLAG_SHARE_DELETE);
  if (!file.IsValid())
    return false;

  // The expansion of |path| into a full path may make it longer.
  constexpr int kMaxPathLength = MAX_PATH + 10;
  wchar_t native_file_path[kMaxPathLength];
  // kMaxPathLength includes space for trailing '\0' so we subtract 1.
  // Returned length, used_wchars, does not include trailing '\0'.
  // Failure is indicated by returning 0 or >= kMaxPathLength.
  DWORD used_wchars = ::GetFinalPathNameByHandleW(
      file.GetPlatformFile(), native_file_path, kMaxPathLength - 1,
      FILE_NAME_NORMALIZED | VOLUME_NAME_NT);

  if (used_wchars >= kMaxPathLength || used_wchars == 0)
    return false;

  // GetFinalPathNameByHandle() returns the \\?\ syntax for file names and
  // existing code expects we return a path starting 'X:\' so we call
  // DevicePathToDriveLetterPath rather than using VOLUME_NAME_DOS above.
  return DevicePathToDriveLetterPath(
      FilePath(FilePath::StringPieceType(native_file_path, used_wchars)),
      real_path);
}

bool DevicePathToDriveLetterPath(const FilePath& nt_device_path,
                                 FilePath* out_drive_letter_path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  // Get the mapping of drive letters to device paths.
  const int kDriveMappingSize = 1024;
  wchar_t drive_mapping[kDriveMappingSize] = {'\0'};
  if (!::GetLogicalDriveStringsW(kDriveMappingSize - 1, drive_mapping)) {
    CR_DLOG(Error) << "Failed to get drive mapping.";
    return false;
  }

  // The drive mapping is a sequence of null terminated strings.
  // The last string is empty.
  wchar_t* drive_map_ptr = drive_mapping;
  wchar_t device_path_as_string[MAX_PATH];
  wchar_t drive[] = FILE_PATH_LITERAL(" :");

  // For each string in the drive mapping, get the junction that links
  // to it.  If that junction is a prefix of |device_path|, then we
  // know that |drive| is the real path prefix.
  while (*drive_map_ptr) {
    drive[0] = drive_map_ptr[0];  // Copy the drive letter.

    if (::QueryDosDeviceW(drive, device_path_as_string, MAX_PATH)) {
      FilePath device_path(device_path_as_string);
      if (device_path == nt_device_path ||
          device_path.IsParent(nt_device_path)) {
        *out_drive_letter_path =
            FilePath(drive + nt_device_path.value().substr(
                                 wcslen(device_path_as_string)));
        return true;
      }
    }
    // Move to the next drive letter string, which starts one
    // increment after the '\0' that terminates the current string.
    while (*drive_map_ptr++) {}
  }

  // No drive matched.  The path does not start with a device junction
  // that is mounted as a drive letter.  This means there is no drive
  // letter path to the volume that holds |device_path|, so fail.
  return false;
}

FilePath MakeLongFilePath(const FilePath& input) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  DWORD path_long_len = ::GetLongPathNameW(input.value().c_str(), nullptr, 0);
  if (path_long_len == 0UL)
    return FilePath();

  std::wstring path_long_str;
  path_long_len = ::GetLongPathNameW(input.value().c_str(),
                                     WriteInto(&path_long_str, path_long_len),
                                     path_long_len);
  if (path_long_len == 0UL)
    return FilePath();

  return FilePath(path_long_str);
}

bool CreateWinHardLink(const FilePath& to_file, const FilePath& from_file) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return ::CreateHardLinkW(to_file.value().c_str(), from_file.value().c_str(),
                           nullptr);
}

bool GetFileInfo(const FilePath& file_path, File::Info* results) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  WIN32_FILE_ATTRIBUTE_DATA attr;
  if (!GetFileAttributesExW(file_path.value().c_str(), GetFileExInfoStandard,
                            &attr)) {
    return false;
  }

  ULARGE_INTEGER size;
  size.HighPart = attr.nFileSizeHigh;
  size.LowPart = attr.nFileSizeLow;
  results->size = size.QuadPart;

  results->is_directory =
      (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  results->last_modified = Time::FromFileTime(&attr.ftLastWriteTime);
  results->last_accessed = Time::FromFileTime(&attr.ftLastAccessTime);
  results->creation_time = Time::FromFileTime(&attr.ftCreationTime);

  return true;
}

int ReadFile(const FilePath& filename, char* data, int max_size) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  win::ScopedHandle file(::CreateFileW(filename.value().c_str(), GENERIC_READ,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                       OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN,
                                       NULL));
  if (!file.IsValid())
    return -1;

  DWORD read;
  if (::ReadFile(file.Get(), data, max_size, &read, NULL))
    return read;

  return -1;
}

int WriteFile(const FilePath& filename, const char* data, int size) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  win::ScopedHandle file(::CreateFileW(filename.value().c_str(), GENERIC_WRITE, 
                                       0, NULL, CREATE_ALWAYS, 
                                       FILE_ATTRIBUTE_NORMAL, NULL));
  if (!file.IsValid()) {
    CR_DPLOG(Warning) << "CreateFile failed for path " << filename.value();
    return -1;
  }

  DWORD written;
  BOOL result = ::WriteFile(file.Get(), data, size, &written, NULL);
  if (result && static_cast<int>(written) == size)
    return written;

  if (!result) {
    // WriteFile failed.
    CR_DPLOG(Warning) << "writing file " << filename.value() << " failed";
  } else {
    // Didn't write all the bytes.
    CR_DLOG(Warning) << "wrote" << written << " bytes to " << filename.value()
                     << " expected " << size;
  }
  return -1;
}

bool AppendToFile(const FilePath& filename, const char* data, int size) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  win::ScopedHandle file(::CreateFileW(filename.value().c_str(), 
                                       FILE_APPEND_DATA, 0, NULL, 
                                       OPEN_EXISTING, 0, NULL));
  if (!file.IsValid()) {
    CR_DPLOG(Warning) << "CreateFile failed for path " << filename.value();
    return false;
  }

  DWORD written;
  BOOL result = ::WriteFile(file.Get(), data, size, &written, NULL);
  if (result && static_cast<int>(written) == size)
    return true;

  if (!result) {
    // WriteFile failed.
    CR_DPLOG(Warning) << "Writing file " << filename.value() << " failed";
  } else {
    // Didn't write all the bytes.
    CR_DPLOG(Warning) << "Only wrote " << written << " out of " << size 
                      << " byte(s) to " << filename.value();
  }
  return false;
}

// Gets the current working directory for the process.
bool GetCurrentDirectory(FilePath* path) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  wchar_t system_buffer[MAX_PATH];
  system_buffer[0] = 0;
  DWORD len = ::GetCurrentDirectoryW(MAX_PATH, system_buffer);
  if (len == 0 || len > MAX_PATH)
    return false;
  // TODO(evanm): the old behavior of this function was to always strip the
  // trailing slash.  We duplicate this here, but it shouldn't be necessary
  // when everyone is using the appropriate FilePath APIs.
  *path = FilePath(FilePath::StringPieceType(system_buffer))
              .StripTrailingSeparators();
  return true;
}

// Sets the current working directory for the process.
bool SetCurrentDirectory(const FilePath& directory) {
  ///ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return ::SetCurrentDirectoryW(directory.value().c_str()) != 0;
}

}  // namesapce cr
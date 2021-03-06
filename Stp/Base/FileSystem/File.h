// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILE_H_
#define STP_BASE_FS_FILE_H_

#include "Base/Containers/Buffer.h"
#include "Base/Error/SystemErrorCode.h"
#include "Base/FileSystem/FilePath.h"
#include "Base/Time/Time.h"

#if OS(WIN)
#include "Base/Win/ScopedHandle.h"
#include "Base/Win/WindowsHeader.h"
#endif

namespace stp {

class FileInfo;

#if OS(WIN)
using NativeFile = HANDLE;
const NativeFile InvalidNativeFile = INVALID_HANDLE_VALUE;
#elif OS(POSIX)
using NativeFile = int;
const NativeFile InvalidNativeFile = -1;
#endif // OS(*)

enum class FileAccess {
  ReadOnly,
  WriteOnly,
  ReadWrite,
};

enum class FileMode {
  Create,
  CreateNew,
  OpenExisting,
  OpenOrCreate,
  OpenTruncated,
  Append,
};

class BASE_EXPORT File {
  STATIC_ONLY(File);
 public:
  // Return true if the given path exists on the local filesystem.
  static bool exists(const FilePath& path) WARN_UNUSED_RESULT;

  // Fetch information about the given file path.
  static void getInfo(const FilePath& path, FileInfo& info);
  static SystemErrorCode tryGetInfo(const FilePath& path, FileInfo& info) WARN_UNUSED_RESULT;

  // Return an absolute version of a relative path.
  // On POSIX, this function fails if the path does not exist.
  // This function can result in I/O so it can be slow.
  static FilePath makeAbsolutePath(const FilePath& input);
  static SystemErrorCode tryMakeAbsolutePath(const FilePath& input, FilePath& output) WARN_UNUSED_RESULT;

  // Delete an existing file.
  static void remove(const FilePath& path);
  static SystemErrorCode tryRemove(const FilePath& path) WARN_UNUSED_RESULT;

  #if OS(WIN)
  static SystemErrorCode tryMakeLongPath(const FilePath& input, FilePath& output) WARN_UNUSED_RESULT;
  // Schedule to delete the given path, whether it's a file or an empty directory,
  // until the operating system is restarted.
  // The file/directory to be deleted should exist in a temp folder.
  static SystemErrorCode tryRemoveAfterReboot(const FilePath& path) WARN_UNUSED_RESULT;
  #endif

  // Rename file |from| to |to|.
  // Both paths must be on the same volume, or the function will fail.
  // Destination file will be created if it doesn't exist.
  // On Windows it preserves attributes of the target file.
  static void replace(const FilePath& from, const FilePath& to);
  static SystemErrorCode tryReplace(const FilePath& from, const FilePath& to) WARN_UNUSED_RESULT;

  static Buffer readAll(const FilePath& path);
  static void writeAll(const FilePath& path, BufferSpan input);

  static String readAllText(const FilePath& path, StringSpan codec = StringSpan());
  static void writeAllText(const FilePath& path, StringSpan text, StringSpan codec = StringSpan());

  // Creates a temporary file. The full path is returned on success.
  // The file will be empty and all handles closed after this function returns.
  // This function does NOT unlink the file.
  // In CreateTemporary() variant system-wide temporary directory is used.
  static FilePath createTemporary();

  static FilePath createTemporaryIn(const FilePath& dir);
  static SystemErrorCode tryCreateTemporaryIn(const FilePath& dir, FilePath& output_path) WARN_UNUSED_RESULT;

  static void writeAtomically(const FilePath& path, BufferSpan input);

  #if OS(POSIX)
  // Creates a symbolic link at |symlink| pointing to |target|.
  static void createSymbolicLink(const FilePath& symlink, const FilePath& target);
  static SystemErrorCode tryCreateSymbolicLink(const FilePath& symlink, const FilePath& target) WARN_UNUSED_RESULT;

  static FilePath readSymbolicLink(const FilePath& symlink);
  static SystemErrorCode tryReadSymbolicLink(const FilePath& symlink, FilePath& out_target) WARN_UNUSED_RESULT;

  // Get/Set the permissions of the given |path|.
  // If |path| is symbolic link, |mode| is the permission of a file which the symlink points to.
  static int getPosixPermissions(const FilePath& path);
  static SystemErrorCode tryGetPosixPermissions(const FilePath& path, int& out_mode) WARN_UNUSED_RESULT;

  static void setPosixPermissions(const FilePath& path, int mode);
  static SystemErrorCode trySetPosixPermissions(const FilePath& path, int mode) WARN_UNUSED_RESULT;
  #endif
};

} // namespace stp

#endif // STP_BASE_FS_FILE_H_

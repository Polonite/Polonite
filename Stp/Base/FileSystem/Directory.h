// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_DIRECTORY_H_
#define STP_BASE_FS_DIRECTORY_H_

#include "Base/Error/SystemErrorCode.h"
#include "Base/FileSystem/FilePath.h"
#include "Base/Util/Flags.h"

namespace stp {

class BASE_EXPORT Directory {
  STATIC_ONLY(Directory);
 public:
  // Returns true if the given path exists and the path points to a directory.
  static bool exists(const FilePath& path);

  static void create(const FilePath& path);
  static SystemErrorCode tryCreate(const FilePath& path);

  static void removeEmpty(const FilePath& path);
  static SystemErrorCode tryRemoveEmpty(const FilePath& path);

  // Creates a directory, as well as creating any parent directories, if they don't exist.
  // Returns 'true' on successful creation, or if the directory already exists.
  static void createPath(const FilePath& path);
  static SystemErrorCode tryCreatePath(const FilePath& path);

  // Deletes a file or directory (recursively).
  // EQUIVALENT TO "rm -rf", SO USE WITH CAUTION.
  static void removeRecursively(const FilePath& path);

  // Returns the total number of bytes used by all the files under |root_path|.
  static uint64_t computeSize(const FilePath& path);

  // Retrieves the disk space in bytes on the volume containing |path|.
  struct DriveSpaceInfo {
    int64_t total;     // total size, in bytes
    int64_t free;      // total free space available on drive, in bytes
    int64_t available; // similar to free, but takes disk quotas into account
  };
  static DriveSpaceInfo getDriveSpaceInfo(const FilePath& path);
  static SystemErrorCode tryGetDriveSpaceInfo(const FilePath& path, DriveSpaceInfo& out_info);
};

} // namespace stp

#endif // STP_BASE_FS_DIRECTORY_H_

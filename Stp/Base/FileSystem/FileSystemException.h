// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_FS_FILESYSTEMEXCEPTION_H_
#define STP_BASE_FS_FILESYSTEMEXCEPTION_H_

#include "Base/FileSystem/FilePath.h"
#include "Base/Error/SystemException.h"

namespace stp {

class BASE_EXPORT FileSystemException : public SystemException {
 public:
  explicit FileSystemException(SystemErrorCode error_code) noexcept
      : SystemException(error_code) {}

  FileSystemException(SystemErrorCode error_code, FilePathSpan path)
      : SystemException(error_code), path_(path) {}

  FileSystemException(SystemErrorCode error_code, FilePathSpan path, FilePathSpan aux_path)
      : SystemException(error_code), path_(path), aux_path_(aux_path) {}

  const FilePath& getPath() const { return path_; }
  const FilePath& getAuxPath() const { return aux_path_; }

  StringSpan getName() const noexcept override;

 protected:
  void onFormat(TextWriter& out) const final;

 private:
  FilePath path_;
  FilePath aux_path_;
};

} // namespace stp

#endif // STP_BASE_FS_FILESYSTEMEXCEPTION_H_

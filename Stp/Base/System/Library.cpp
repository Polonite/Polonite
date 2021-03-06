// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Library.h"

namespace stp {

bool Library::TryLoad(const FilePath& path, LibraryLoadError* out_error) {
  Reset();

  NativeLibrary native = TryLoadNative(toNullTerminated(path), out_error);
  if (native == NullNativeLibrary)
    return false;
  Reset(native);
  return true;
}

} // namespace stp

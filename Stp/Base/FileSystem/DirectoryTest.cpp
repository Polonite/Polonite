// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/FileSystem/Directory.h"

#include "Base/FileSystem/KnownPaths.h"
#include "Base/Test/GTest.h"

namespace stp {
namespace {

TEST(DirectoryTest, getDriveSpaceInfo) {
  // We aren't actually testing that it's correct, just that it's sane.
  FilePath tmp_path = getTempDirPath();
  ASSERT_FALSE(tmp_path.isEmpty());

  auto space = Directory::getDriveSpaceInfo(tmp_path);
  EXPECT_GT(space.available, 0);
  EXPECT_GT(space.total, 0);
  EXPECT_GT(space.free, 0);
}

} // namespace
} // namespace stp

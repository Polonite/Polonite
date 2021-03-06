// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Test/PerfTestSuite.h"

#include "Base/Debug/Debugger.h"
#include "Base/FileSystem/KnownPaths.h"
#include "Base/Process/CommandLine.h"
#include "Base/Test/GTest.h"
#include "Base/Test/PerfLog.h"

namespace stp {

void PerfTestSuite::onDidInit() {
  TestSuite::onDidInit();

  auto& command_line = CommandLine::forCurrentProcess();

  // Initialize the perf timer log
  FilePath log_path;
  if (!command_line.tryGet("log-file", log_path)) {
    #if OS(ANDROID)
    FilePath tmp_dir = GetAppCachePath();
    log_path = combineFilePaths(tmp_dir, log_path.GetFileName());
    #else
    log_path = getExecutableFilePath();
    #endif
    log_path.ChangeExtension(".log");
  }
  ASSERT_TRUE(InitPerfLog(log_path));
}

void PerfTestSuite::onWillFini() {
  TestSuite::onWillFini();

  FinalizePerfLog();
}

} // namespace stp

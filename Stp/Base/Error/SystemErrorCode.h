// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_SYSTEMERRORCODE_H_
#define STP_BASE_ERROR_SYSTEMERRORCODE_H_

#include "Base/Compiler/Os.h"

#if OS(WIN)
#include "Base/Win/WinErrorCode.h"
#elif OS(POSIX)
#include "Base/Posix/PosixErrorCode.h"
#endif

namespace stp {

#if OS(WIN)
using SystemErrorCode = WinErrorCode;
#elif OS(POSIX)
using SystemErrorCode = PosixErrorCode;
#endif

} // namespace stp

#endif // STP_BASE_ERROR_SYSTEMERRORCODE_H_

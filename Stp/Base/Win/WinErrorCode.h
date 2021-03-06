// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_WIN_WINERRORCODE_H_
#define STP_BASE_WIN_WINERRORCODE_H_

#include "Base/Error/ErrorCode.h"
#include "Base/Win/WindowsHeader.h"

namespace stp {

enum class WinErrorCode : DWORD {
  Success = ERROR_SUCCESS,
  FileNotFound = ERROR_FILE_NOT_FOUND,
  PathNotFound = ERROR_PATH_NOT_FOUND,
  TooManyOpenFiles = ERROR_TOO_MANY_OPEN_FILES,
  AccessDenied = ERROR_ACCESS_DENIED,
  InvalidHandle = ERROR_INVALID_HANDLE,

  NoMoreFiles = ERROR_NO_MORE_FILES,

  AlreadyExists = ERROR_ALREADY_EXISTS,
};

inline bool isOk(WinErrorCode code) noexcept { return LIKELY(code == WinErrorCode::Success); }

inline WinErrorCode getLastWinErrorCode() noexcept {
  return static_cast<WinErrorCode>(::GetLastError());
}

BASE_EXPORT const ErrorCategory* GetWinErrorCategory();

inline ErrorCode makeErrorCode(WinErrorCode code) noexcept {
  return ErrorCode(static_cast<int>(code), GetWinErrorCategory());
}

namespace detail {
BASE_EXPORT void format(TextWriter& out, WinErrorCode code);
}

inline void format(TextWriter& out, WinErrorCode code, const StringSpan& opts) {
  detail::format(out, code);
}
inline TextWriter& operator<<(TextWriter& out, WinErrorCode code) {
  detail::format(out, code); return out;
}

} // namespace stp

#endif // STP_BASE_WIN_WINERRORCODE_H_

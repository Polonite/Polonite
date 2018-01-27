// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_POSIX_POSIXERRORCODE_H_
#define STP_BASE_POSIX_POSIXERRORCODE_H_

#include "Base/Error/ErrorCode.h"

#include <errno.h>

namespace stp {

enum class PosixErrorCode : int {
  Ok = 0,
  OperationNotPermitted = EPERM,
  NoSuchFileOrDirectory = ENOENT,
  FileExists = EEXIST,
};

template<> struct TIsErrorCodeEnumTmpl<PosixErrorCode> : TTrue {};

inline bool IsOk(PosixErrorCode code) { return code == PosixErrorCode::Ok; }

inline PosixErrorCode GetLastPosixErrorCode() { return static_cast<PosixErrorCode>(errno); }

BASE_EXPORT const ErrorCategory* GetPosixErrorCategory() noexcept;

inline ErrorCode MakeErrorCode(PosixErrorCode code) noexcept {
  return ErrorCode(static_cast<int>(code), GetPosixErrorCategory());
}

namespace detail {
BASE_EXPORT void Format(TextWriter& out, PosixErrorCode code);
}

inline void Format(TextWriter& out, PosixErrorCode code, const StringSpan& opts) {
  detail::Format(out, code);
}
inline TextWriter& operator<<(TextWriter& out, PosixErrorCode code) {
  detail::Format(out, code); return out;
}

} // namespace stp

#endif // STP_BASE_POSIX_POSIXERRORCODE_H_
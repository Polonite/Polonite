// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/StringReader.h"

#include "Base/Text/Utf.h"

namespace stp {

int32_t StringReader::onPeek() {
  return onReadInternal(false);
}

int32_t StringReader::onRead() {
  return onReadInternal(true);
}

inline int32_t StringReader::onReadInternal(bool advance) {
  int size = string_.size();
  if (size == 0)
    return -1;

  const char* it = string_.data();
  char32_t uc = Utf8::TryDecode(it, it + size);
  if (unicode::IsDecodeError(uc))
    return -1;

  if (advance)
    string_.removePrefix(it - string_.data());

  return static_cast<int32_t>(uc);
}

int StringReader::OnRead(char* dst, int count) {
  ASSERT(count >= 0);

  if (count > string_.size())
    count = string_.size();

  if (count > 0) {
    uninitializedCopy(dst, string_.data(), count);
    string_.removePrefix(count);
  }
  return count;
}

} // namespace stp

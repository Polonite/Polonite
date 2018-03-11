// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/StringSpan.h"

#include "Base/Text/AsciiChar.h"
#include "Base/Text/Utf.h"
#include "Base/Type/Comparable.h"

namespace stp {

int StringSpan::indexOfUnit(char c) const {
  for (int i = 0; i < length_; ++i) {
    if (data_[i] == c)
      return i;
  }
  return -1;
}

int StringSpan::lastIndexOfUnit(char c) const {
  for (int i = length_ - 1; i >= 0; --i) {
    if (data_[i] == c)
      return i;
  }
  return -1;
}

int StringSpan::indexOfRune(char32_t rune) const {
  if (isAscii(rune))
    return indexOfUnit(static_cast<char>(rune));
  char encoded[Utf8::MaxEncodedRuneLength];
  int n = EncodeUtf(encoded, rune);
  return indexOf(StringSpan(encoded, n));
}

int StringSpan::lastIndexOfRune(char32_t rune) const {
  if (isAscii(rune))
    return lastIndexOfUnit(static_cast<char>(rune));
  char encoded[Utf8::MaxEncodedRuneLength];
  int n = EncodeUtf(encoded, rune);
  return lastIndexOf(StringSpan(encoded, n));
}

int StringSpan::indexOf(const StringSpan& needle) const {
  auto* haystack_data = data_;

  auto* needle_data = needle.data_;
  int needle_length = needle.length_;

  int delta = length_ - needle_length;

  unsigned search_hash = 0;
  unsigned match_hash = 0;

  for (int i = 0; i < needle_length; ++i) {
    search_hash += static_cast<byte_t>(haystack_data[i]);
    match_hash += static_cast<byte_t>(needle_data[i]);
  }
  int i = 0;
  while (search_hash != match_hash || !equalObjects(haystack_data + i, needle_data, needle_length)) {
    if (i == delta)
      return -1;
    search_hash += static_cast<byte_t>(haystack_data[i + needle_length]);
    search_hash -= static_cast<byte_t>(haystack_data[i]);
    ++i;
  }
  return i;
}

int StringSpan::lastIndexOf(const StringSpan& needle) const {
  auto* haystack_data = data_;

  auto* needle_data = needle.data_;
  int needle_length = needle.length_;

  int delta = length_ - needle_length;

  unsigned search_sum = 0;
  unsigned match_sum = 0;

  for (int i = 0; i < needle_length; ++i) {
    search_sum += static_cast<byte_t>(haystack_data[i]);
    match_sum += static_cast<byte_t>(needle_data[i]);
  }
  while (search_sum != match_sum || !equalObjects(haystack_data + delta, needle_data, needle_length)) {
    if (delta == 0)
      return -1;
    --delta;
    search_sum += static_cast<byte_t>(haystack_data[delta + needle_length]);
    search_sum -= static_cast<byte_t>(haystack_data[delta]);
  }
  return delta;
}

bool operator==(const StringSpan& lhs, const StringSpan& rhs) {
  if (lhs.length() != rhs.length())
    return false;
  if (!lhs.isEmpty())
    return ::memcmp(lhs.data(), rhs.data(), lhs.length()) == 0;
  return true;
}

int compare(const StringSpan& lhs, const StringSpan& rhs) {
  int common_length = min(lhs.length(), rhs.length());
  if (common_length) {
    int rv = ::memcmp(lhs.data(), rhs.data(), toUnsigned(common_length));
    if (rv)
      return rv;
  }
  return compare(lhs.length(), rhs.length());
}

} // namespace stp

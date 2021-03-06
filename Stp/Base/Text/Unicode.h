// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_UNICODE_H_
#define STP_BASE_TEXT_UNICODE_H_

#include "Base/Debug/Assert.h"

namespace stp {
namespace unicode {

constexpr char16_t ReplacementRune = 0xFFFD;
constexpr char32_t MaxRune = 0x10FFFFu;

constexpr char16_t MinLeadSurrogate = 0xD800u;
constexpr char16_t MaxLeadSurrogate = 0xDBFFu;
constexpr char16_t MinTrailSurrogate = 0xDC00u;
constexpr char16_t MaxTrailSurrogate = 0xDFFFu;
constexpr char32_t SurrogateOffset = (MinLeadSurrogate << 10u) + MinTrailSurrogate - 0x10000u;

static constexpr char32_t EndOfStreamRune = 0x80000000u;
static constexpr char32_t InvalidSequenceRune = 0x80000001u;

inline bool IsDecodeError(char32_t c) { return (c & 0x80000000u) != 0; }

// Excludes the surrogate code units [0xD800, 0xDFFF] and
// code-points larger than 0x10FFFF (the highest code-point allowed).
// Non-characters and unassigned code-points are allowed.
inline bool IsValidRune(char32_t rune) {
  return rune < MinLeadSurrogate ||
        (rune > MaxTrailSurrogate && rune <= MaxRune);
}

// Excludes non-characters [U+FDD0..U+FDEF], and all code-points ending in
// 0xFFFE or 0xFFFF) from the set of valid characters.
inline bool IsValidCharacter(char32_t rune) {
  return rune < MinLeadSurrogate ||
        (rune > MaxTrailSurrogate && rune < 0xFDD0u) ||
        (rune > 0xFDEFu && rune <= MaxRune && (rune & 0xFFFEu) != 0xFFFEu);
}

// Accepts code units in [U+D8000..U+DFFF] range.
inline bool IsSurrogate(char32_t code) {
  return (code & 0xFFFFF800) == 0xD800;
}

// Accepts code units in [U+D8000..U+DBFF] range.
inline bool IsLeadSurrogate(char32_t code) {
  return (code & 0xFFFFFC00) == 0xD800;
}

// Accepts code units in [U+DC000..U+DFFF] range.
inline bool IsTrailSurrogate(char32_t code) {
  return (code & 0xFFFFFC00) == 0xDC00;
}

// Checks whether given surrogate is leading.
// False is returned for trailing surrogate.
inline bool SurrogateIsLeading(char32_t surrogate) {
  ASSERT(IsSurrogate(surrogate));
  return (surrogate & 0x400) == 0;
}

// Decodes a code-point from surrogate pair.
// Always succeeds (asserts for valid lead and trail surrogate).
inline char32_t DecodeSurrogatePair(char32_t lead, char32_t trail) {
  ASSERT(IsLeadSurrogate(lead));
  ASSERT(IsTrailSurrogate(trail));
  return (lead << 10u) + trail - SurrogateOffset;
}

} // namespace unicode
} // namespace stp

#endif // STP_BASE_TEXT_UNICODE_H_

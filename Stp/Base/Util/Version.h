// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_VERSION_H_
#define STP_BASE_UTIL_VERSION_H_

#include "Base/Containers/InlineList.h"
#include "Base/Type/Parsable.h"

namespace stp {

class Version;

BASE_EXPORT bool tryParse(StringSpan text, Version& version);

// Version represents a dotted version number, like "1.2.3.4", supporting
// parsing and comparison.
class BASE_EXPORT Version {
 public:
  using PartType = int;

  Version() = default;
  Version(PartType major_part, PartType minor_part, PartType micro_part)
      : parts_({major_part, minor_part, micro_part}) {}

  PartType getMajor() const { return getPartAt(0); }
  PartType getMinor() const { return getPartAt(1); }
  PartType getMicro() const { return getPartAt(2); }

  void setPart(int at, PartType value);
  PartType getPartAt(int at) const { return at < parts_.size() ? parts_[at] : 0; }

  bool operator==(const Version& other) const { return compareTo(other) == 0; }
  bool operator!=(const Version& other) const { return compareTo(other) != 0; }
  bool operator<=(const Version& other) const { return compareTo(other) <= 0; }
  bool operator>=(const Version& other) const { return compareTo(other) >= 0; }
  bool operator< (const Version& other) const { return compareTo(other) <  0; }
  bool operator> (const Version& other) const { return compareTo(other) >  0; }
  friend int compare(const Version& l, const Version& r) { return l.compareTo(r); }
  friend HashCode partialHash(const Version& x) { return x.hashImpl(); }

  friend TextWriter& operator<<(TextWriter& out, const Version& x) {
    x.formatImpl(out); return out;
  }
  friend void format(TextWriter& out, const Version& x, const StringSpan& opts) {
    x.formatImpl(out);
  }

  friend bool tryParse(StringSpan text, Version& out);

 private:
  int compareTo(const Version& other) const;
  void formatImpl(TextWriter& out) const;
  HashCode hashImpl() const;

  InlineList<PartType, 4> parts_;
};

} // namespace stp

#endif // STP_BASE_UTIL_VERSION_H_

// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONSTRINGBUILDER_H_
#define STP_BASE_JSON_JSONSTRINGBUILDER_H_

#include "Base/Containers/List.h"

namespace stp {

// A helper class used for parsing strings. One optimization performed is to
// create Value with a StringSpan to avoid unnecessary String copies.
// This is not possible if the input string needs to be decoded from
// UTF-16 to UTF-8, or if an escape sequence causes characters to be skipped.
// This class centralizes that logic.
class BASE_EXPORT JsonStringBuilder {
 public:
  JsonStringBuilder() {}
  ~JsonStringBuilder();

  JsonStringBuilder(const JsonStringBuilder& other);
  JsonStringBuilder& operator=(const JsonStringBuilder& other);

  JsonStringBuilder(JsonStringBuilder&& other);
  JsonStringBuilder& operator=(JsonStringBuilder&& other);

  JsonStringBuilder& operator=(StringSpan other);

  // For creating a reference to read-only data.
  explicit JsonStringBuilder(const char* pos) : data_(const_cast<char*>(pos)) {}

  StringSpan toSpan() const { return StringSpan(data_, size_); }

  bool isEmpty() const { return size_ == 0; }

  ALWAYS_INLINE const char* data() const { return data_; }
  ALWAYS_INLINE int size() const { return size_; }

  bool operator==(const JsonStringBuilder& other) const { return toSpan() == other.toSpan(); }
  bool operator!=(const JsonStringBuilder& other) const { return !operator==(other); }

  // Either increases the |length_| of the string or copies the character if
  // the JsonStringBuilder has been converted. |c| must be in the basic ASCII
  // plane; all other characters need to be in UTF-8 units, appended with
  // appendString below.
  void append(char c);

  char* appendUninitialized(int n);

  // Must be Convert()ed to use.
  void appendString(StringSpan string);

  void appendInPlace(const char* str, int size);

  // Converts the builder from its default StringSpan to a full String,
  // performing a copy.
  void Convert();

  bool OwnsData() const { return capacity_ >= 0; }

  friend void swap(JsonStringBuilder& lhs, JsonStringBuilder& rhs) {
    swap(lhs.data_, rhs.data_);
    swap(lhs.size_, rhs.size_);
    swap(lhs.capacity_, rhs.capacity_);
  }

 private:
  int recommendCapacity(int n);

  char* data_ = nullptr;
  int size_ = 0;
  int capacity_ = -1;
};

inline JsonStringBuilder::~JsonStringBuilder() {
  if (capacity_ > 0)
    Free(data_);
}

} // namespace stp

#endif // STP_BASE_JSON_JSONSTRINGBUILDER_H_

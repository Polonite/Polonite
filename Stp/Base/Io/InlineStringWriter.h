// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_INLINESTRINGWRITER_H_
#define STP_BASE_IO_INLINESTRINGWRITER_H_

#include "Base/Containers/InlineList.h"
#include "Base/Io/TextWriter.h"

namespace stp {

class BASE_EXPORT InlineStringWriter final : public TextWriter {
 public:
  explicit InlineStringWriter(InlineListBase<char>* string) : string_(*string) {}

  TextEncoding GetEncoding() const override;

 protected:
  void OnWriteAsciiChar(char c) override;
  void OnWriteUnicodeChar(char32_t c) override;
  void OnWriteAscii(StringSpan text) override;
  void OnWriteUtf8(StringSpan text) override;
  void OnWriteUtf16(String16Span text) override;
  void OnIndent(int count, char c) override;

 private:
  InlineListBase<char>& string_;
};

} // namespace stp

#endif // STP_BASE_IO_INLINESTRINGWRITER_H_
